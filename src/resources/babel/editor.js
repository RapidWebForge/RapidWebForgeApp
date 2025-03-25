const fs = require("fs");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;
const prettier = require("prettier");

const [, , filePath, operation, referenceId, position, payload] = process.argv;

(async () => {
  try {
    const sourceCode = fs.readFileSync(filePath, "utf-8");

    const ast = parser.parse(sourceCode, {
      sourceType: "module",
      plugins: ["jsx", "typescript"],
    });

    let fragmentAst = null;
    try {
      fragmentAst =
        payload && payload !== '""'
          ? parser.parseExpression(JSON.parse(payload), { plugins: ["jsx"] })
          : null;
    } catch (e) {
      console.error("❌ Error parsing payload:", e.message);
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
            // Eliminar React.lazy(() => import("./views/ViewName"))
            if (
              path.node.callee.type === "MemberExpression" &&
              path.node.callee.property.name === "lazy" &&
              path.node.arguments[0]?.body?.body[0]?.argument?.value?.includes(
                importName,
              )
            ) {
              path.remove();
              modified = true;
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

    const formattedCode = await prettier.format(output.code, {
      parser: "babel-ts",
      semi: true,
      singleQuote: false,
      trailingComma: "es5",
      tabWidth: 2,
    });

    fs.writeFileSync(filePath, formattedCode);
    console.log("✅ Code successfully modified and formatted.");
  } catch (err) {
    console.error("❌ Unexpected error:", err.message);
    process.exit(1);
  }
})();
