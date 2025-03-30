const fs = require("fs");
const { execSync } = require("child_process");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;

const [, , filePath, operation, referenceId, ...rest] = process.argv;

(async () => {
  try {
    const sourceCode = fs.readFileSync(filePath, "utf-8");

    const ast = parser.parse(sourceCode, {
      sourceType: "module",
      plugins: ["jsx", "typescript"],
    });

    let position = "";
    let payloadPath = "";

    if (operation === "modify") {
      payloadPath = rest[0] || "";
    } else if (operation === "insert") {
      position = rest[0] || "";
      payloadPath = rest[1] || "";
    } else if (operation === "refactor-delete") {
      position = rest[0] || "";
    }

    let fragmentAst = null;
    if (operation !== "delete" && operation !== "refactor-delete")
      try {
        const raw = fs.readFileSync(payloadPath, "utf-8");
        fragmentAst = parser.parseExpression(JSON.parse(raw), {
          plugins: ["jsx"],
        });
      } catch (e) {
        console.error("❌ Error parsing payload:", e.message);
        fragmentAst = null;
      }

    let modified = false;

    if (operation === "modify" || operation === "delete") {
      traverse(ast, {
        JSXElement(path) {
          const attr = path.node.openingElement.attributes.find(
            (a) => a.type === "JSXAttribute" && a.name.name === "data-id",
          );
          if (!attr || attr.value.value !== referenceId) return;

          if (operation === "modify" && fragmentAst) {
            path.replaceWith(fragmentAst);
          } else if (operation === "delete") {
            path.remove();
          }

          modified = true;
          path.stop();
        },
      });
    }

    if (operation === "refactor-delete" && referenceId) {
      const importName = referenceId;

      if (position === "View") {
        // Refactor para vistas (views)
        traverse(ast, {
          ImportDeclaration(path) {
            const importPath = path.node.source.value;
            if (importPath.includes(importName)) {
              path.remove();
              modified = true;
            }
          },
          CallExpression(path) {
            const isLazy =
              path.node.callee.type === "MemberExpression" &&
              path.node.callee.object.name === "React" &&
              path.node.callee.property.name === "lazy";

            if (!isLazy) return;

            const arg = path.node.arguments[0];
            if (
              arg.type === "ArrowFunctionExpression" &&
              arg.body.type === "CallExpression" &&
              arg.body.callee.type === "Import"
            ) {
              const parentVarDecl = path.findParent((p) =>
                p.isVariableDeclaration(),
              );
              if (parentVarDecl) {
                parentVarDecl.remove(); // Eliminar toda la declaración: const View = React.lazy(...)
                modified = true;
              } else {
                path.remove(); // fallback por si no encuentra el contenedor
                modified = true;
              }
            }
          },
          JSXElement(path) {
            // Eliminar <Route path="/" element={<ViewName />} />
            if (
              path.node.openingElement.name.name === "Route" &&
              path.node.openingElement.attributes.some(
                (attr) =>
                  attr.type === "JSXAttribute" &&
                  attr.name.name === "element" &&
                  attr.value?.expression?.openingElement?.name?.name ===
                    importName,
              )
            ) {
              path.remove();
              modified = true;
            }
          },
        });
      } else if (position === "CustomComponent") {
        // Refactor para componentes personalizados
        traverse(ast, {
          ImportDeclaration(path) {
            const importPath = path.node.source.value;
            if (importPath.includes(importName)) {
              path.remove();
              modified = true;
            }
          },
          JSXElement(path) {
            // Eliminar <ComponentName />
            if (
              path.node.openingElement.name.type === "JSXIdentifier" &&
              path.node.openingElement.name.name === importName
            ) {
              path.remove();
              modified = true;
            }
          },
        });
      } else {
        console.error("❌ Unknown refactor-delete position:", position);
      }
    }

    if (operation === "insert" && fragmentAst) {
      // Detectar si se trata de un componente personalizado
      const insertedComponentName =
        fragmentAst.type === "JSXElement" &&
        fragmentAst.openingElement.name.type === "JSXIdentifier"
          ? fragmentAst.openingElement.name.name
          : null;

      const alreadyImported = insertedComponentName
        ? ast.program.body.some(
            (node) =>
              node.type === "ImportDeclaration" &&
              node.specifiers.some(
                (spec) =>
                  spec.type === "ImportDefaultSpecifier" &&
                  spec.local.name === insertedComponentName,
              ),
          )
        : false;

      if (insertedComponentName && !alreadyImported) {
        const importDeclaration = {
          type: "ImportDeclaration",
          specifiers: [
            {
              type: "ImportDefaultSpecifier",
              local: {
                type: "Identifier",
                name: insertedComponentName,
              },
            },
          ],
          source: {
            type: "StringLiteral",
            value: `../components/${insertedComponentName}`,
          },
        };

        ast.program.body.unshift(importDeclaration);
        modified = true;
      }

      // Inserción normal
      if (referenceId === "none" && position === "inner") {
        traverse(ast, {
          JSXElement(path) {
            if (path.node.openingElement.name.name === "div" && !modified) {
              path.node.children.push(fragmentAst);
              modified = true;
              path.stop();
            }
          },
        });
      } else {
        traverse(ast, {
          JSXElement(path) {
            const attr = path.node.openingElement.attributes.find(
              (a) => a.type === "JSXAttribute" && a.name.name === "data-id",
            );
            if (!attr || attr.value.value !== referenceId) return;

            switch (position) {
              case "before":
                path.insertBefore(fragmentAst);
                break;
              case "after":
                path.insertAfter(fragmentAst);
                break;
              case "inner":
                path.node.children.push(fragmentAst);
                break;
              default:
                console.error("❌ Unknown insert position:", position);
                return;
            }

            modified = true;
            path.stop();
          },
        });
      }
    }

    const output = generate(ast, { retainLines: true }, sourceCode);

    // Guardar el código antes de formatear
    fs.writeFileSync(filePath, output.code);

    // Formatear con Biome (requiere que esté en node_modules/.bin o accesible desde bun)
    process.env.PATH = `${process.cwd()}/node_modules/.bin:${process.env.PATH}`;
    execSync(`biome format ${filePath} --write`, { stdio: "inherit" });

    console.log("✅ Code successfully modified and formatted.");
  } catch (err) {
    console.error("❌ Unexpected error:", err.message);
    process.exit(1);
  }
})();
