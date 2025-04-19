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
      // Obtenemos la ruta absoluta
      const payloadPath = path.isAbsolute(transactionName)
        ? transactionName
        : path.resolve(process.cwd(), transactionName);
      const payload = JSON.parse(fs.readFileSync(payloadPath, "utf-8"));
      const modelName = payload.name;
      const lowerModelName =
        modelName.charAt(0).toLowerCase() + modelName.slice(1);
      const fields = payload.fields;

      const filesToModify = [
        path.join(basePath, "backend", "models", modelName + ".js"),
        path.join(
          basePath,
          "backend",
          "controllers",
          modelName + "Controller.js",
        ),
        path.join(
          basePath,
          "frontend",
          "src",
          "models",
          lowerModelName + ".ts",
        ),
        path.join(basePath, "backend", "routes", lowerModelName + "Routes.js"),
      ];

      for (const filePath of filesToModify) {
        const source = fs.readFileSync(filePath, "utf-8");
        let fileModified = false;
        let errorMessage = "",
          successMessage = "";
        let ast;

        if (filePath.includes("models") && filePath.includes("backend")) {
          ast = parser.parse(source, {
            sourceType: "module",
            plugins: ["jsx", "javascript"],
          });

          traverse(ast, {
            CallExpression(path) {
              // Detectar sequelize.define("Tasks", ATTRS, OPTIONS)
              if (
                t.isMemberExpression(path.node.callee) &&
                t.isIdentifier(path.node.callee.object, {
                  name: "sequelize",
                }) &&
                t.isIdentifier(path.node.callee.property, { name: "define" }) &&
                path.node.arguments.length >= 2 &&
                t.isStringLiteral(path.node.arguments[0], { value: modelName })
              ) {
                // 1. Construir las propiedades de atributos desde payload.fields
                const attrProps = fields.map((field) => {
                  const props = [];

                  // type: DataTypes.<TYPE>
                  props.push(
                    t.objectProperty(
                      t.identifier("type"),
                      t.memberExpression(
                        t.identifier("DataTypes"),
                        t.identifier(field.type),
                      ),
                    ),
                  );

                  if (field.isForeignKey) {
                    // si es foreignKey, injectar references: { model: 'tabla', key: 'id' }
                    const foreignKeyTableLower =
                      field.foreignKeyTable.charAt(0).toLowerCase() +
                      field.foreignKeyTable.slice(1);
                    props.push(
                      t.objectProperty(
                        t.identifier("references"),
                        t.objectExpression([
                          t.objectProperty(
                            t.identifier("model"),
                            t.stringLiteral(foreignKeyTableLower),
                          ),
                          t.objectProperty(
                            t.identifier("key"),
                            t.stringLiteral("id"),
                          ),
                        ]),
                      ),
                    );
                  } else {
                    // allowNull: <boolean invertido de isNull>
                    if (field.isNull)
                      props.push(
                        t.objectProperty(
                          t.identifier("allowNull"),
                          t.booleanLiteral(field.isNull),
                        ),
                      );

                    if (field.isUnique)
                      props.push(
                        t.objectProperty(
                          t.identifier("unique"),
                          t.booleanLiteral(field.isUnique),
                        ),
                      );

                    // NOT SUPPORTED YET
                    /* if (field.hasDefault)
                      props.push(
                        t.objectProperty(
                          t.identifier("defaultValue"),
                          t.booleanLiteral(field.hasDefault),
                        ),
                      ); */
                  }

                  return t.objectProperty(
                    t.identifier(field.name),
                    t.objectExpression(props),
                  );
                });

                // 2. Reemplazar el objeto de atributos completo
                path.node.arguments[1] = t.objectExpression(attrProps);
              }
            },
            ReturnStatement(path) {
              // Detectar `return <ModelName>;`
              if (t.isIdentifier(path.node.argument, { name: modelName })) {
                // Para cada campo marcado como isForeignKey, generar el bloque de associate
                payload.fields
                  .filter((field) => field.isForeignKey)
                  .forEach((field) => {
                    const assocNode = template.statement.ast(`
                    ${modelName}.associate = (models) => {
                      ${modelName}.belongsTo(models.${field.foreignKeyTable}, {
                        foreignKey: '${field.name}', // LLave foránea
                        onDelete: 'SET NULL',
                        onUpdate: 'CASCADE',
                      });
                    };
                  `);
                    path.insertBefore(assocNode);
                  });

                path.stop(); // no insertar más veces
              }
            },
          });

          fileModified = true;
          successMessage = `✅ ${filePath} actualizado con nuevos campos.`;
          errorMessage = `i No se modificó ${filePath}: no se encontró sequelize.define("${modelName}")`;
        }
        if (filePath.includes("models") && filePath.includes("frontend")) {
          // Leer y parsear el archivo TS
          ast = parser.parse(source, {
            sourceType: "module",
            plugins: ["typescript", "jsx"],
          });

          traverse(ast, {
            TSInterfaceDeclaration(path) {
              // Solo nos interesa la interfaz Tasks (payload.name)
              if (path.node.id.name === modelName) {
                // Reconstruir los miembros de la interfaz con payload.fields
                const members = payload.fields.map((field) => {
                  // Mapear el tipo de Sequelize a TS
                  let tsTypeNode;
                  switch (field.type) {
                    case "STRING":
                      tsTypeNode = t.tsStringKeyword();
                      break;
                    case "BOOLEAN":
                      tsTypeNode = t.tsBooleanKeyword();
                      break;
                    default:
                      tsTypeNode = t.tsAnyKeyword();
                  }
                  return t.tsPropertySignature(
                    t.identifier(field.name),
                    t.tsTypeAnnotation(tsTypeNode),
                  );
                });

                // Reemplazar el array de miembros
                path.node.body.body = members;
                fileModified = true;
                path.stop();
              }
            },
          });

          successMessage = `✅ Interfaz actualizada en ${filePath}`;
          errorMessage = `i No se encontró la interfaz ${modelName} en ${filePath}`;
        }
        if (filePath.includes("controllers")) {
          ast = parser.parse(source, {
            sourceType: "module",
            plugins: ["jsx", "javascript"],
          });

          // 0) Eliminar asignaciones antiguas: lowerModelName.<field> = <field>;
          traverse(ast, {
            ExpressionStatement(path) {
              const expr = path.node.expression;
              if (
                t.isAssignmentExpression(expr) &&
                t.isMemberExpression(expr.left) &&
                t.isIdentifier(expr.left.object, { name: lowerModelName }) &&
                payload.fields.some((f) => expr.left.property.name === f.name)
              ) {
                path.remove();
                fileModified = true;
              }
            },
          });

          traverse(ast, {
            // 1) Actualizar `const { … } = req.body;`
            VariableDeclarator(path) {
              if (
                t.isObjectPattern(path.node.id) &&
                t.isCallExpression(path.node.init) &&
                t.isIdentifier(path.node.init.callee, { name: "require" }) &&
                path.node.init.arguments.length === 1 &&
                t.isStringLiteral(path.node.init.arguments[0], {
                  value: "../models",
                })
              ) {
                const props = path.node.id.properties;

                // Asegurar el modelo principal
                if (!props.some((p) => p.key.name === modelName)) {
                  props.push(
                    t.objectProperty(
                      t.identifier(modelName),
                      t.identifier(modelName),
                      false,
                      true,
                    ),
                  );
                }

                // Agregar cada tabla foránea
                payload.fields
                  .filter((f) => f.isForeignKey)
                  .forEach((field) => {
                    const fk = field.foreignKeyTable;
                    if (!props.some((p) => p.key.name === fk)) {
                      props.push(
                        t.objectProperty(
                          t.identifier(fk),
                          t.identifier(fk),
                          false,
                          true,
                        ),
                      );
                      fileModified = true;
                    }
                  });
              }

              if (
                t.isObjectPattern(path.node.id) &&
                t.isMemberExpression(path.node.init) &&
                t.isIdentifier(path.node.init.object, { name: "req" }) &&
                t.isIdentifier(path.node.init.property, { name: "body" })
              ) {
                path.node.id.properties = payload.fields.map((f) =>
                  t.objectProperty(
                    t.identifier(f.name),
                    t.identifier(f.name),
                    false,
                    true,
                  ),
                );
              }
            },

            // 2) Reemplazar el argumento de `.create({ … })`
            CallExpression(path) {
              const callee = path.node.callee;
              if (
                t.isMemberExpression(callee) &&
                t.isIdentifier(callee.object, { name: modelName }) &&
                t.isIdentifier(callee.property, { name: "create" })
              ) {
                path.node.arguments = [
                  t.objectExpression(
                    payload.fields.map((f) =>
                      t.objectProperty(
                        t.identifier(f.name),
                        t.identifier(f.name),
                        false,
                        true,
                      ),
                    ),
                  ),
                ];
              }
            },

            // 3) Ajustar las asignaciones en update: lowerModelName.<field> = <field>;
            ExpressionStatement(path) {
              // Detectamos la llamada `await lowerModelName.save()`
              if (
                t.isAwaitExpression(path.node.expression) &&
                t.isCallExpression(path.node.expression.argument) &&
                t.isMemberExpression(path.node.expression.argument.callee) &&
                t.isIdentifier(path.node.expression.argument.callee.object, {
                  name: lowerModelName,
                }) &&
                t.isIdentifier(path.node.expression.argument.callee.property, {
                  name: "save",
                })
              ) {
                // Por cada campo del payload, creamos y metemos la asignación
                payload.fields.forEach((field) => {
                  const assign = t.expressionStatement(
                    t.assignmentExpression(
                      "=",
                      t.memberExpression(
                        t.identifier(lowerModelName),
                        t.identifier(field.name),
                      ),
                      t.identifier(field.name),
                    ),
                  );
                  path.insertBefore(assign);
                });
                fileModified = true;
              }
            },

            AssignmentExpression(path) {
              const isCreate =
                t.isMemberExpression(path.node.left) &&
                t.isIdentifier(path.node.left.object, { name: "exports" }) &&
                t.isIdentifier(path.node.left.property, {
                  name: `create${modelName}`,
                });

              const isDelete =
                t.isMemberExpression(path.node.left) &&
                t.isIdentifier(path.node.left.object, { name: "exports" }) &&
                t.isIdentifier(path.node.left.property, {
                  name: `delete${modelName}ById`,
                });

              // 4) FK checks in create<Model>
              if (isCreate) {
                const fn = path.node.right;
                if (
                  t.isFunctionExpression(fn) ||
                  t.isArrowFunctionExpression(fn)
                ) {
                  const tryStmt = fn.body.body.find((stmt) =>
                    t.isTryStatement(stmt),
                  );
                  if (tryStmt) {
                    const innerBody = tryStmt.block.body;
                    const idx = innerBody.findIndex(
                      (stmt) =>
                        t.isVariableDeclaration(stmt) &&
                        stmt.declarations.some(
                          (d) =>
                            t.isObjectPattern(d.id) &&
                            t.isMemberExpression(d.init) &&
                            t.isIdentifier(d.init.object, { name: "req" }) &&
                            t.isIdentifier(d.init.property, { name: "body" }),
                        ),
                    );

                    if (idx !== -1) {
                      payload.fields
                        .filter((f) => f.isForeignKey)
                        .forEach((field) => {
                          const fkLower =
                            field.foreignKeyTable.charAt(0).toLowerCase() +
                            field.foreignKeyTable.slice(1);

                          const alreadyExists = innerBody.some(
                            (stmt) =>
                              t.isVariableDeclaration(stmt) &&
                              generate(stmt).code.includes(
                                `await ${field.foreignKeyTable}.findByPk`,
                              ),
                          );

                          if (!alreadyExists) {
                            const stmts = template.ast(`
                              // Verificar si la transacción relacionada ${field.foreignKeyTable} existe
                              const ${fkLower} = await ${field.foreignKeyTable}.findByPk(${field.name});
                              if (!${fkLower}) {
                                return res.status(404).json({ error: '${field.foreignKeyTable} not found' });
                              }
                            `);
                            innerBody.splice(idx + 1, 0, ...stmts);
                            fileModified = true;
                          }
                        });
                    }
                  }
                }
              }

              // 5) Insertar export getAll<Model>For<ForeignTable>
              if (isDelete) {
                payload.fields
                  .filter((f) => f.isForeignKey)
                  .forEach((field) => {
                    const funcName = `getAll${modelName}For${field.foreignKeyTable}`;
                    const already = ast.program.body.some(
                      (node) =>
                        t.isExpressionStatement(node) &&
                        generate(node).code.includes(`exports.${funcName}`),
                    );
                    if (!already) {
                      const fkLower =
                        field.foreignKeyTable.charAt(0).toLowerCase() +
                        field.foreignKeyTable.slice(1);
                      const stmt = template.statement.ast(`
                        exports.${funcName} = async (req, res) => {
                          try {
                            const ${fkLower} = await ${field.foreignKeyTable}.findByPk(req.params.id, {
                              include: [${modelName}]
                            });
                            if (${fkLower}) {
                              res.json(${fkLower}.${modelName}s);
                            } else {
                              res.status(404).json({ error: '${field.foreignKeyTable} not found' });
                            }
                          } catch (error) {
                            res.status(500).json({ error: error.message });
                          }
                        };
                      `);
                      path.parentPath.insertAfter(stmt);
                      fileModified = true;
                    }
                  });

                path.stop(); // Solo detener si era el bloque de delete
              }
            },
          });

          fileModified = true;
          successMessage = `✅ ${filePath} actualizado con nuevos campos.`;
          errorMessage = `i No se modificó ${filePath}`;
        }
        if (filePath.includes("routes")) {
          ast = parser.parse(source, {
            sourceType: "module",
            plugins: ["jsx", "javascript"],
          });

          traverse(ast, {
            AssignmentExpression(path) {
              if (
                t.isMemberExpression(path.node.left) &&
                t.isIdentifier(path.node.left.object, { name: "module" }) &&
                t.isIdentifier(path.node.left.property, { name: "exports" }) &&
                t.isIdentifier(path.node.right, { name: "router" })
              ) {
                // Por cada campo FK, añadimos una ruta
                payload.fields
                  .filter((field) => field.isForeignKey)
                  .forEach((field) => {
                    const fkTableLower =
                      field.foreignKeyTable.charAt(0).toLowerCase() +
                      field.foreignKeyTable.slice(1);
                    const routeStmt = template.statement.ast(`
                    router.get(
                      '/${fkTableLower}/:id/${lowerModelName}s',
                      ${lowerModelName}Controller.getAll${modelName}For${field.foreignKeyTable}
                    );
                    `);
                    path.insertBefore(routeStmt);
                  });

                path.stop(); // dejamos de buscar más
              }
            },
          });

          fileModified = true;
          successMessage = `✅ ${filePath} actualizado con rutas para llaves foráneas.`;
          errorMessage = `i No se modificó ${filePath}`;
        }

        if (fileModified) {
          const output = generate(ast, { retainLines: true }, source);
          fs.writeFileSync(filePath, output.code);
          execSync(`biome format ${filePath} --write`, { stdio: "inherit" });
          console.log(successMessage);
        } else {
          console.log(errorMessage);
        }
      }
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
          console.log(`i No se hicieron cambios en ${filePath}`);
        }
      }
    }

    console.log("✅ Code successfully modified and formatted.");
  } catch (err) {
    console.error("❌ Unexpected error:", err.message);
    process.exit(1);
  }
})();
