const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;
const template = require("@babel/template").default;
const t = require("@babel/types");

const [, , basePath, operation, transactionName] = process.argv;

(async () => {
  try {
    let modified = false;

    if (operation === "modify") {
      //modify
    }
    if ((operation === "insert" || operation === "delete") && transactionName) {
      const capitalized =
        transactionName.charAt(0).toUpperCase() + transactionName.slice(1);
      const lower =
        transactionName.charAt(0).toLowerCase() + transactionName.slice(1);
      const modelImportName = capitalized;
      const modelImportPath = `./${capitalized}`;
      const routeImportName = `${lower}Routes`;
      const routeImportPath = `./${lower}Routes`;

      const filesToModify = [
        path.join(basePath, "routes", "index.js"),
        path.join(basePath, "models", "index.js"),
      ];

      for (const filePath of filesToModify) {
        const sourceCode = fs.readFileSync(filePath, "utf-8");

        const ast = parser.parse(sourceCode, {
          sourceType: "module",
          plugins: ["jsx", "typescript"],
        });

        let modified = false;

        if (filePath.includes("models/index.js")) {
          if (operation === "insert") {
            // 1. Verificar si ya existe el import del mdoelo
            const importExists = ast.program.body.some(
              (node) =>
                t.isVariableDeclaration(node) &&
                node.declarations.some(
                  (decl) =>
                    t.isCallExpression(decl.init) &&
                    t.isCallExpression(decl.init.callee) &&
                    t.isIdentifier(decl.init.callee.callee, {
                      name: "require",
                    }) &&
                    decl.init.callee.arguments.length === 1 &&
                    decl.init.callee.arguments[0].value === modelImportPath &&
                    decl.init.arguments.length === 1 &&
                    t.isIdentifier(decl.init.arguments[0], {
                      name: "sequelize",
                    }),
                ),
            );

            if (!importExists) {
              // 2. Recorrer el AST hasta encontrar `module.exports = { ... }`
              traverse(ast, {
                AssignmentExpression(path) {
                  if (
                    t.isMemberExpression(path.node.left) &&
                    t.isIdentifier(path.node.left.object, { name: "module" }) &&
                    t.isIdentifier(path.node.left.property, {
                      name: "exports",
                    }) &&
                    t.isObjectExpression(path.node.right)
                  ) {
                    // 3. Crear y insertar el import justo antes de module.exports
                    const importNode = template.statement.ast(
                      `const ${modelImportName} = require("${modelImportPath}")(sequelize);`,
                    );
                    path.insertBefore(importNode);

                    // 4. Añadir Modelo al objeto de exports si no está ya
                    const exportsObj = path.node.right;
                    const alreadyExported = exportsObj.properties.some(
                      (prop) =>
                        t.isObjectProperty(prop) &&
                        t.isIdentifier(prop.key, { name: modelImportName }),
                    );
                    if (!alreadyExported) {
                      exportsObj.properties.push(
                        t.objectProperty(
                          t.identifier(modelImportName),
                          t.identifier(modelImportName),
                          false,
                          true,
                        ),
                      );
                    }

                    modified = true;
                    path.stop();
                  }
                },
              });
            }
          }

          if (operation === "delete") {
            traverse(ast, {
              VariableDeclaration(path) {
                const code = generate(path.node).code;
                if (code.includes(`require("${modelImportPath}")`)) {
                  path.remove();
                  modified = true;
                }
              },
              ObjectExpression(path) {
                path.node.properties = path.node.properties.filter((prop) => {
                  if (
                    prop.type === "ObjectProperty" &&
                    prop.key.name === modelImportName
                  ) {
                    modified = true;
                    return false;
                  }
                  return true;
                });
              },
            });
          }
        }

        if (filePath.includes("routes/index.js")) {
          if (operation === "insert") {
            // 1. Preparar nombres
            const importExists = ast.program.body.some(
              (node) =>
                t.isVariableDeclaration(node) &&
                node.declarations.some(
                  (decl) =>
                    t.isIdentifier(decl.id, { name: routeImportName }) &&
                    t.isCallExpression(decl.init) &&
                    t.isIdentifier(decl.init.callee, { name: "require" }) &&
                    decl.init.arguments[0].value === routeImportPath,
                ),
            );

            // 2. Insertar el import justo después de `const router = express.Router();`
            if (!importExists) {
              traverse(ast, {
                VariableDeclaration(path) {
                  // Detectar `const router = express.Router();`
                  if (
                    path.node.declarations.some(
                      (decl) =>
                        t.isIdentifier(decl.id, { name: "router" }) &&
                        t.isCallExpression(decl.init) &&
                        t.isMemberExpression(decl.init.callee) &&
                        t.isIdentifier(decl.init.callee.object, {
                          name: "express",
                        }) &&
                        t.isIdentifier(decl.init.callee.property, {
                          name: "Router",
                        }),
                    )
                  ) {
                    const importNode = template.statement.ast(
                      `const ${routeImportName} = require("${routeImportPath}");`,
                    );
                    path.insertAfter(importNode);
                    modified = true;
                    path.stop();
                  }
                },
              });
            }

            // 3. Comprobar si ya existe router.use(routes);
            const useExists = ast.program.body.some(
              (node) =>
                t.isExpressionStatement(node) &&
                t.isCallExpression(node.expression) &&
                t.isMemberExpression(node.expression.callee) &&
                t.isIdentifier(node.expression.callee.object, {
                  name: "router",
                }) &&
                t.isIdentifier(node.expression.callee.property, {
                  name: "use",
                }) &&
                node.expression.arguments.some((arg) =>
                  t.isIdentifier(arg, { name: routeImportName }),
                ),
            );

            // 4. Insertar `router.use(...)` justo antes de `module.exports = router;`
            if (!useExists) {
              traverse(ast, {
                AssignmentExpression(path) {
                  if (
                    t.isMemberExpression(path.node.left) &&
                    t.isIdentifier(path.node.left.object, { name: "module" }) &&
                    t.isIdentifier(path.node.left.property, {
                      name: "exports",
                    }) &&
                    t.isIdentifier(path.node.right, { name: "router" })
                  ) {
                    const useNode = template.statement.ast(
                      `router.use(${routeImportName});`,
                    );
                    path.insertBefore(useNode);
                    modified = true;
                    path.stop();
                  }
                },
              });
            }
          }

          if (operation === "delete") {
            traverse(ast, {
              VariableDeclaration(path) {
                const code = generate(path.node).code;
                if (code.includes(`require("${routeImportPath}")`)) {
                  path.remove();
                  modified = true;
                }
              },
              ExpressionStatement(path) {
                const code = generate(path.node).code;
                if (code.includes(`router.use(${routeImportName})`)) {
                  path.remove();
                  modified = true;
                }
              },
            });
          }
        }

        if (modified) {
          const output = generate(ast, { retainLines: true }, sourceCode);
          fs.writeFileSync(filePath, output.code);
          execSync(`biome format ${filePath} --write`, { stdio: "inherit" });
          console.log(`✅ ${filePath} actualizado.`);
        } else {
          console.log(`ℹ️ No se hicieron cambios en ${filePath}`);
        }
      }
    }

    console.log("✅ Code successfully modified and formatted.");
  } catch (err) {
    console.error("❌ Unexpected error:", err.message);
    process.exit(1);
  }
})();
