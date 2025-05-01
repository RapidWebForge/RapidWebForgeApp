const fs = require("fs");
const { execSync } = require("child_process");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;
const t = require("@babel/types");
const template = require("@babel/template").default;

const [, , filePath, operation, referenceId, ...rest] = process.argv;

const getAttrValue = (node, attrName) => {
  const attr = node.openingElement.attributes.find(
    (a) => a.type === "JSXAttribute" && a.name.name === attrName,
  );
  return attr && attr.value && attr.value.value;
};
const toLower = (s) => s.charAt(0).toLowerCase() + s.slice(1);

const toCapitalize = (str) => {
  if (!str) return "";
  return str.charAt(0).toUpperCase() + str.slice(1).toLowerCase();
};

const fileName = filePath.split("/").pop().split(".")[0];

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
    } else if (operation === "refactor-delete" || operation === "create") {
      position = rest[0] || "";
    }

    const checkMissingImports = (model) => {
      const serviceImportExists = ast.program.body.some(
        (node) =>
          node.type === "ImportDeclaration" &&
          node.source.value === `../services/${model}Service`,
      );

      const importDeclIndex = ast.program.body.findIndex(
        (node) =>
          t.isImportDeclaration(node) &&
          node.source.value === `../models/${model}`,
      );

      if (importDeclIndex !== -1) {
        const importDecl = ast.program.body[importDeclIndex];

        const hasModel = importDecl.specifiers.some(
          (spec) => t.isImportSpecifier(spec) && spec.imported.name === model,
        );
        const hasDefaults = importDecl.specifiers.some(
          (spec) =>
            t.isImportSpecifier(spec) && spec.imported.name === "defaults",
        );

        if (!hasModel) {
          importDecl.specifiers.push(
            t.importSpecifier(t.identifier(model), t.identifier(model)),
          );
        }
        if (!hasDefaults) {
          importDecl.specifiers.push(
            t.importSpecifier(
              t.identifier("defaults"),
              t.identifier("defaults"),
            ),
          );
        }
      } else {
        const newImport = t.importDeclaration(
          [
            t.importSpecifier(t.identifier(model), t.identifier(model)),
            t.importSpecifier(
              t.identifier("defaults"),
              t.identifier("defaults"),
            ),
          ],
          t.stringLiteral(`../models/${model}`),
        );
        ast.program.body.unshift(newImport);
      }

      if (!serviceImportExists) {
        const svcImport = t.importDeclaration(
          [t.importDefaultSpecifier(t.identifier(`${model}Service`))],
          t.stringLiteral(`../services/${model}Service`),
        );
        ast.program.body.unshift(svcImport);
      }
    };

    const ensureReactHooksImport = (ast, hooks = []) => {
      const reactImport = ast.program.body.find(
        (node) =>
          node.type === "ImportDeclaration" && node.source.value === "react",
      );

      if (reactImport) {
        const existingSpecifiers = new Set(
          reactImport.specifiers.map((s) =>
            s.type === "ImportSpecifier" ? s.imported.name : null,
          ),
        );

        const missingHooks = hooks.filter(
          (hook) => !existingSpecifiers.has(hook),
        );

        // Agregar los hooks faltantes como importaciones con nombre
        if (missingHooks.length > 0) {
          reactImport.specifiers.push(
            ...missingHooks.map((hook) => ({
              type: "ImportSpecifier",
              imported: { type: "Identifier", name: hook },
              local: { type: "Identifier", name: hook },
            })),
          );
        }
      } else {
        // No hay importación de React, se agrega todo desde cero
        ast.program.body.unshift({
          type: "ImportDeclaration",
          source: { type: "StringLiteral", value: "react" },
          specifiers: [
            {
              type: "ImportDefaultSpecifier",
              local: { type: "Identifier", name: "React" },
            },
            ...hooks.map((hook) => ({
              type: "ImportSpecifier",
              imported: { type: "Identifier", name: hook },
              local: { type: "Identifier", name: hook },
            })),
          ],
        });
      }
    };

    let fragmentAst = null;
    if (
      operation !== "delete" &&
      operation !== "refactor-delete" &&
      operation !== "create"
    )
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
            // Normalización previa (como te mostré antes)
            const oldModelRaw = getAttrValue(path.node, "data-rwf-model");
            const newModelRaw = getAttrValue(fragmentAst, "data-rwf-model");
            const oldMethodRaw = getAttrValue(path.node, "data-rwf-method");
            const newMethodRaw = getAttrValue(fragmentAst, "data-rwf-method");

            const oldModel = oldModelRaw?.trim() ? oldModelRaw : null;
            const newModel = newModelRaw?.trim() ? newModelRaw : null;
            const oldMethod = oldMethodRaw?.trim() ? oldMethodRaw : null;
            const newMethod = newMethodRaw?.trim() ? newMethodRaw : null;

            const hasOldModel = oldModel !== null;
            const hasNewModel = newModel !== null;
            const hasOldMethod = oldMethod !== null;
            const hasNewMethod = newMethod !== null;

            const modelWasReplaced =
              hasOldModel && hasNewModel && oldModel !== newModel;
            const modelWasRemoved = hasOldModel && !hasNewModel;
            const modelWasAdded = hasNewModel && !hasOldModel;
            const methodWasReplaced =
              hasOldMethod && hasNewMethod && oldMethod !== newMethod;
            const methodWasRemoved = hasOldMethod && !hasNewMethod;
            const methodWasAdded = hasNewMethod && !hasOldMethod;

            const isDiv = path.node.openingElement.name.name === "div";
            const isForm = path.node.openingElement.name.name === "form";

            if (isDiv) {
              // Eliminar useState y useEffect antiguos del oldModel
              if ((modelWasReplaced || modelWasRemoved) && hasOldModel) {
                traverse(ast, {
                  VariableDeclaration(path) {
                    const code = generate(path.node).code;
                    // p.ej. const [tasks, setTasks] = useState<Tasks[]>([]);
                    if (
                      code.includes(
                        `const [${toLower(oldModel)}, set${oldModel}]`,
                      ) &&
                      code.includes(`useState<${oldModel}[]>`)
                    ) {
                      path.remove();
                      modified = true;
                    }
                  },
                  ExpressionStatement(path) {
                    const code = generate(path.node).code;
                    // p.ej. useEffect(() => { TasksService.getAllTasks()... }, []);
                    if (
                      code.includes("useEffect") &&
                      code.includes(`${oldModel}Service.getAll${oldModel}()`)
                    ) {
                      path.remove();
                      modified = true;
                    }
                  },
                });
              }

              // Agregar nuevas definiciones
              if ((modelWasReplaced || modelWasAdded) && hasNewModel) {
                const lowerNewModel = toLower(newModel);
                const stateCode = `
                  const [${lowerNewModel}, set${newModel}] = useState<${newModel}[]>([]);
                      `.trim();
                const effectCode = `
                  useEffect(() => {
                    ${newModel}Service.getAll${newModel}()
                      .then(response => set${newModel}(response))
                      .catch(error => console.error("Error fetching ${newModel} data:", error));
                  }, []);
                      `.trim();

                traverse(ast, {
                  FunctionDeclaration(path) {
                    if (path.node.id?.name === fileName) {
                      // Insertamos primero el effect, luego el state, para mantener orden lógico
                      const effectNode = template.ast(effectCode, {
                        plugins: ["jsx", "typescript"],
                      });
                      const stateNode = template.ast(stateCode, {
                        plugins: ["jsx", "typescript"],
                      });
                      path.node.body.body.unshift(stateNode);
                      path.node.body.body.unshift(effectNode);
                      modified = true;
                    }
                  },
                });
              }
            }
            if (isForm) {
              // Eliminar useState, handleChange y handleSubmit del oldModel
              if (
                modelWasReplaced ||
                modelWasRemoved ||
                methodWasReplaced ||
                methodWasRemoved
              ) {
                if (hasOldModel && hasOldMethod) {
                  const capOldMtd = toCapitalize(oldMethod);
                  const lowOldMtd = oldMethod.toLowerCase();

                  traverse(ast, {
                    VariableDeclaration(path) {
                      const code = generate(path.node).code;

                      // Remover useState
                      if (
                        code.includes(
                          `const [${lowOldMtd}${oldModel}, set${capOldMtd}${oldModel}]`,
                        ) &&
                        code.includes(
                          `useState<${oldModel}>(defaults.default${capOldMtd}${oldModel})`,
                        )
                      ) {
                        path.remove();
                        modified = true;
                      }

                      // Remover handleChange
                      if (
                        code.includes(
                          `const handleChange${capOldMtd}${oldModel}`,
                        ) &&
                        code.includes(`set${capOldMtd}${oldModel}`) &&
                        code.includes("[name]: value")
                      ) {
                        path.remove();
                        modified = true;
                      }

                      // Remover handleSubmit
                      if (
                        code.includes(
                          `const handleSubmit${capOldMtd}${oldModel}`,
                        ) &&
                        code.includes(`${oldModel}Service.`)
                      ) {
                        path.remove();
                        modified = true;
                      }
                    },
                  });
                }
              }

              // Agregar nuevas definiciones
              if (
                modelWasReplaced ||
                modelWasAdded ||
                methodWasReplaced ||
                methodWasAdded
              ) {
                // Ni intentar si no hay newModel u newMethod
                if (hasNewModel && hasNewMethod) {
                  const capNewMtd = toCapitalize(newMethod);
                  const lowNewMtd = newMethod.toLowerCase();

                  // Determinar el nombre de la acción del servicio
                  let methodService = null;
                  if (newMethod === "PUT") methodService = "update";
                  if (newMethod === "POST") methodService = "create";

                  const stateCode = `
                    const [${lowNewMtd}${newModel}, set${capNewMtd}${newModel}] = useState<${newModel}>(defaults.default${capNewMtd}${newModel});
                    `.trim();

                  const handleChangeCode = `
                    const handleChange${capNewMtd}${newModel} = (e: any) => {
                      const { name, value } = e.target;
                      set${capNewMtd}${newModel}((prev) => ({
                        ...prev,
                        [name]: value,
                      }));
                    };
                    `.trim();

                  const handleSubmitCode = `
                    const handleSubmit${capNewMtd}${newModel} = async (e: React.FormEvent) => {
                      e.preventDefault();
                      if (!${lowNewMtd}${newModel}) {
                        console.error("Data is undefined");
                        return;
                      }
                      try {
                        const response = await ${newModel}Service.${methodService}${newModel}(${lowNewMtd}${newModel});
                        console.log("Form submitted successfully:", response);
                      } catch (error) {
                        console.error("Error submitting form:", error);
                      }
                    };
                    `.trim();

                  traverse(ast, {
                    FunctionDeclaration(path) {
                      if (path.node.id?.name === fileName) {
                        const stateNode = template.ast(stateCode, {
                          plugins: ["jsx", "typescript"],
                        });
                        const changeNode = template.ast(handleChangeCode, {
                          plugins: ["jsx", "typescript"],
                        });
                        const submitNode = template.ast(handleSubmitCode, {
                          plugins: ["jsx", "typescript"],
                        });

                        // Insertamos en orden: state, change, submit
                        path.node.body.body.unshift(submitNode);
                        path.node.body.body.unshift(changeNode);
                        path.node.body.body.unshift(stateNode);
                        modified = true;
                      }
                    },
                  });
                }
              }
            }

            path.replaceWith(t.cloneNode(fragmentAst, true));

            if ((modelWasAdded || modelWasReplaced) && hasNewModel) {
              checkMissingImports(newModel);
              modified = true;
            }

            if (modelWasReplaced || modelWasAdded || modelWasRemoved) {
              const newAst = parser.parse(generate(ast).code, {
                sourceType: "module",
                plugins: ["jsx", "typescript"],
              });

              const defaults = "defaults";
              let usesOldModel = false;
              let usesDefaultModel = false;
              let usesOldModelService = false;

              // 1) Detectar si siguen usándose en el código
              traverse(newAst, {
                Identifier(path) {
                  if (path.findParent((p) => p.isImportDeclaration())) return;
                  const n = path.node.name;
                  if (n === oldModel) usesOldModel = true;
                  if (n === defaults) usesDefaultModel = true;
                  if (n === `${oldModel}Service`) usesOldModelService = true;
                  if (usesOldModel && usesDefaultModel && usesOldModelService)
                    path.stop();
                },
              });

              // 2) Recorrer imports y eliminar solo lo obsoleto
              traverse(ast, {
                ImportDeclaration(path) {
                  const src = path.node.source.value;

                  // 2.1) Servicio
                  if (
                    src === `../services/${oldModel}Service` &&
                    !usesOldModelService
                  ) {
                    path.remove();
                    modified = true;
                    return;
                  }

                  // 2.2) Modelo + defaults
                  if (src === `../models/${oldModel}`) {
                    let changed = false;
                    path.node.specifiers = path.node.specifiers.filter(
                      (spec) => {
                        // { Model, defaults }
                        if (t.isImportSpecifier(spec)) {
                          const key = spec.imported.name;
                          if (key === oldModel && !usesOldModel) {
                            changed = true;
                            return false;
                          }
                          if (key === defaults && !usesDefaultModel) {
                            changed = true;
                            return false;
                          }
                        }
                        // import Model from ...
                        if (
                          t.isImportDefaultSpecifier(spec) &&
                          spec.local.name === oldModel &&
                          !usesOldModel
                        ) {
                          changed = true;
                          return false;
                        }
                        return true;
                      },
                    );

                    // Si quedó vacío, borramos toda la declaración
                    if (path.node.specifiers.length === 0) {
                      path.remove();
                      changed = true;
                    }

                    if (changed) {
                      modified = true;
                    }
                  }
                },
              });
            }

            if (modelWasAdded)
              ensureReactHooksImport(ast, ["useState", "useEffect"]);
          } else if (operation === "delete") {
            let deletedComponentName = null;

            if (path.node.openingElement.name.type === "JSXIdentifier") {
              deletedComponentName = path.node.openingElement.name.name;
            }

            const model = getAttrValue(path.node, "data-rwf-model");
            const method = getAttrValue(path.node, "data-rwf-method");
            const isDiv = path.node.openingElement.name.name === "div";
            const isForm = path.node.openingElement.name.name === "form";

            if (isDiv && model) {
              traverse(ast, {
                VariableDeclaration(path) {
                  const code = generate(path.node).code;
                  if (
                    code.includes(`const [${toLower(model)}, set${model}]`) &&
                    code.includes(`useState<${model}[]>`)
                  ) {
                    path.remove();
                    modified = true;
                  }
                },
                ExpressionStatement(path) {
                  const code = generate(path.node).code;
                  if (
                    code.includes("useEffect") &&
                    code.includes(`${model}Service.getAll${model}()`)
                  ) {
                    path.remove();
                    modified = true;
                  }
                },
              });
            }
            if (isForm && method && model) {
              const capitalizeMethod = toCapitalize(method);
              const lowerMethod = method.toLowerCase();

              traverse(ast, {
                VariableDeclaration(path) {
                  const code = generate(path.node).code;
                  if (
                    code.includes(
                      `const [${lowerMethod}${model}, set${capitalizeMethod}${model}]`,
                    ) &&
                    code.includes(
                      `useState<${model}>(defaults.default${capitalizeMethod}${model})`,
                    )
                  ) {
                    path.remove();
                    modified = true;
                  }
                  if (
                    code.includes("const handleChange") &&
                    code.includes(`set${capitalizeMethod}${model}`) &&
                    code.includes("[name]: value")
                  ) {
                    path.remove();
                    modified = true;
                  }
                  if (
                    code.includes("const handleSubmit") &&
                    code.includes(`${model}Service.`)
                  ) {
                    path.remove();
                    modified = true;
                  }
                },
              });
            }

            path.remove();
            modified = true;

            if (isDiv || isForm) {
              const newAst = parser.parse(generate(ast).code, {
                sourceType: "module",
                plugins: ["jsx", "typescript"],
              });

              const defaults = "defaults";
              let usesOldModel = false;
              let usesDefaultModel = false;
              let usesOldModelService = false;

              // 1) Detectar usos en newAst
              traverse(newAst, {
                Identifier(path) {
                  if (path.findParent((p) => p.isImportDeclaration())) return;
                  const name = path.node.name;
                  if (name === model) usesOldModel = true;
                  if (name === defaults) usesDefaultModel = true;
                  if (name === `${model}Service`) usesOldModelService = true;
                  if (usesOldModel && usesDefaultModel && usesOldModelService) {
                    path.stop();
                  }
                },
              });

              // 2) Limpiar imports obsoletos en ast
              traverse(ast, {
                ImportDeclaration(path) {
                  const src = path.node.source.value;

                  // 2.1) Servicio
                  if (
                    src === `../services/${model}Service` &&
                    !usesOldModelService
                  ) {
                    path.remove();
                    modified = true;
                    return;
                  }

                  // 2.2) Modelo + defaults
                  if (src === `../models/${model}`) {
                    let changed = false;

                    path.node.specifiers = path.node.specifiers.filter(
                      (spec) => {
                        // import { Model, defaults } ...
                        if (t.isImportSpecifier(spec)) {
                          const name = spec.imported.name;
                          if (name === model && !usesOldModel) {
                            changed = true;
                            return false;
                          }
                          if (name === defaults && !usesDefaultModel) {
                            changed = true;
                            return false;
                          }
                        }
                        // import Tasks from ...
                        if (
                          t.isImportDefaultSpecifier(spec) &&
                          spec.local.name === model &&
                          !usesOldModel
                        ) {
                          changed = true;
                          return false;
                        }
                        return true;
                      },
                    );

                    // Si borramos todos los specifiers, eliminar la declaración
                    if (path.node.specifiers.length === 0) {
                      path.remove();
                      changed = true;
                    }

                    if (changed) {
                      modified = true;
                    }
                  }
                },
              });

              // 3) Volver a agregar imports faltantes si fue necesario
              // if (modified) {
              //   checkMissingImports(model);
              // }
            }

            // Ahora, si era un custom component, verificamos si quedan instancias
            if (deletedComponentName) {
              let found = false;

              traverse(ast, {
                JSXElement(path) {
                  if (
                    path.node.openingElement.name.type === "JSXIdentifier" &&
                    path.node.openingElement.name.name === deletedComponentName
                  ) {
                    found = true;
                    path.stop();
                  }
                },
              });

              if (!found) {
                // Si ya no queda ninguna instancia, eliminamos el import
                traverse(ast, {
                  ImportDeclaration(path) {
                    const importPath = path.node.source.value;
                    const isTargetImport = path.node.specifiers.some(
                      (spec) =>
                        spec.type === "ImportDefaultSpecifier" &&
                        spec.local.name === deletedComponentName,
                    );

                    if (
                      importPath.includes(
                        `../components/${deletedComponentName}`,
                      ) &&
                      isTargetImport
                    ) {
                      path.remove();
                      modified = true;
                    }
                  },
                });
              }
            }
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
            // Detectar React.lazy(...)
            if (
              t.isMemberExpression(path.node.callee) &&
              t.isIdentifier(path.node.callee.object, { name: "React" }) &&
              t.isIdentifier(path.node.callee.property, { name: "lazy" })
            ) {
              const [firstArg] = path.node.arguments;
              // Asegurarnos de que es una ArrowFunctionExpression
              if (
                t.isArrowFunctionExpression(firstArg) &&
                t.isCallExpression(firstArg.body) &&
                t.isImport(firstArg.body.callee)
              ) {
                // Verificamos el string literal de la ruta
                const importArg = firstArg.body.arguments[0];
                if (
                  t.isStringLiteral(importArg) &&
                  importArg.value === `./views/${importName}`
                ) {
                  // Eliminamos la declaración completa: const X = React.lazy(...)
                  const varDecl = path.findParent((p) =>
                    p.isVariableDeclaration(),
                  );
                  if (varDecl) {
                    varDecl.remove();
                  } else {
                    path.remove();
                  }
                  modified = true;
                }
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

    if (operation === "create") {
      const viewName = referenceId;
      const routePath = rest[0];

      // 1) Insertar import lazy tras los imports existentes
      traverse(ast, {
        Program(path) {
          // encuentra el índice donde terminan los import
          let lastImport = 0;
          path.node.body.forEach((n, i) => {
            if (t.isImportDeclaration(n)) lastImport = i;
          });
          const importNode = template.statement.ast(
            `const ${viewName} = React.lazy(() => import("./views/${viewName}"));`,
          );
          path.node.body.splice(lastImport + 1, 0, importNode);
          path.stop();
        },
      });

      // 2) Insertar <Route> dentro de <Routes>
      traverse(ast, {
        JSXElement(path) {
          // Detecta <Routes> ... </Routes>
          if (
            t.isJSXIdentifier(path.node.openingElement.name, { name: "Routes" })
          ) {
            // Crear el nuevo nodo <Route path="..." element={<View />} />
            const routeNode = parser.parseExpression(
              `<Route path="${routePath}" element={<${viewName} />} />`,
              { plugins: ["jsx"] },
            );
            // Añadirlo al final de los children de <Routes>
            path.pushContainer("children", routeNode);
            path.stop();
          }
        },
      });
    }

    if (operation === "insert" && fragmentAst) {
      // Detectar si se trata de un componente personalizado
      const insertedComponentName =
        fragmentAst.type === "JSXElement" &&
        fragmentAst.openingElement.name.type === "JSXIdentifier"
          ? fragmentAst.openingElement.name.name
          : null;

      const nativeElements = new Set([
        "div",
        "form",
        "input",
        "span",
        "button",
        "label",
        "select",
        "option",
        "textarea",
        "ul",
        "li",
        "p",
        "h1",
        "h2",
        "h3",
        "h4",
        "h5",
        "h6",
      ]);

      const isCustomComponent =
        insertedComponentName &&
        /^[A-Z]/.test(insertedComponentName) &&
        !nativeElements.has(insertedComponentName);

      const model = getAttrValue(fragmentAst, "data-rwf-model");
      const method = getAttrValue(fragmentAst, "data-rwf-method");

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

      if (isCustomComponent && !alreadyImported) {
        ast.program.body.unshift({
          type: "ImportDeclaration",
          specifiers: [
            {
              type: "ImportDefaultSpecifier",
              local: { type: "Identifier", name: insertedComponentName },
            },
          ],
          source: {
            type: "StringLiteral",
            value: `../components/${insertedComponentName}`,
          },
        });
        modified = true;
      }

      // Model Layout
      if (model && insertedComponentName === "div") {
        checkMissingImports(model);

        const lowerModel = toLower(model);
        const stateCode = `const [${lowerModel}, set${model}] = useState<${model}[]>([]);`;
        const effectCode = `useEffect(() => {
           ${model}Service.getAll${model}()
             .then((response) => {
               set${model}(response);
             })
             .catch((error) => {
               console.error("Error fetching ${model} data:", error);
             });
         }, []);`;

        traverse(ast, {
          FunctionDeclaration(path) {
            if (path.node.id?.name === fileName) {
              // Generar los nodos dentro del callback para que tengan el contexto
              const stateNodeNew = template.ast(stateCode, {
                plugins: ["jsx", "typescript"],
              });
              const effectNodeNew = template.ast(effectCode, {
                plugins: ["jsx", "typescript"],
              });
              // Insertar directamente los nuevos nodos
              path.node.body.body.unshift(stateNodeNew);
              path.node.body.body.unshift(effectNodeNew);
              modified = true;
            }
          },
        });

        modified = true;

        ensureReactHooksImport(ast, ["useState", "useEffect"]);
      }

      // Form
      if (model && insertedComponentName === "form") {
        checkMissingImports(model);
        ensureReactHooksImport(ast, ["useState", "useEffect"]);
      }
      if (method && insertedComponentName === "form") {
        const lowerMethod = method.toLowerCase();
        const capitalizeMethod = toCapitalize(method);

        let methodService = null;

        if (method === "PUT") methodService = "update";
        else if (method === "POST") methodService = "create";

        const formStateCode = `const [${lowerMethod}${model}, set${capitalizeMethod}${model}] = useState<${model}>(defaults.default${capitalizeMethod}${model});`;
        const handleChangeCode = `const handleChange${capitalizeMethod}${model} = (e: any) => {
                 const { name, value } = e.target;
                 set${capitalizeMethod}${model}((prevData) => ({
                   ...prevData,
                   [name]: value,
                 }));
               };`;
        const handleSubmitCode = `const handleSubmit${capitalizeMethod}${model} = async (e: React.FormEvent) => {
                 e.preventDefault();
                 if (!${lowerMethod}${model}) {
                   console.error("Data is undefined");
                   return;
                 }
                 try {
                   const response = await ${model}Service.${methodService}${model}(${lowerMethod}${model});
                   console.log("Form submitted successfully:", response);
                 } catch (error) {
                   console.error("Error submitting form:", error);
                 }
               };`;

        traverse(ast, {
          FunctionDeclaration(path) {
            if (path.node.id?.name === fileName) {
              const stateNodeNew = template.ast(formStateCode, {
                plugins: ["jsx", "typescript"],
              });
              const changeNodeNew = template.ast(handleChangeCode, {
                plugins: ["jsx", "typescript"],
              });
              const submitNodeNew = template.ast(handleSubmitCode, {
                plugins: ["jsx", "typescript"],
              });
              path.node.body.body.unshift(stateNodeNew);
              path.node.body.body.unshift(changeNodeNew);
              path.node.body.body.unshift(submitNodeNew);
              modified = true;
            }
          },
        });

        ensureReactHooksImport(ast, ["useState", "useEffect"]);
      }

      // Inserción normal
      if (referenceId === "none" && position === "inner") {
        // 1) Localizar el último ReturnStatement
        let lastReturnPath = null;
        traverse(ast, {
          ReturnStatement(path) {
            lastReturnPath = path;
          },
        });

        // 2) Si existe, aplicar las dos reglas
        if (lastReturnPath) {
          const ret = lastReturnPath.node;
          const arg = ret.argument;

          // A) Si no había nada: return;
          if (arg == null) {
            ret.argument = t.cloneNode(fragmentAst, true);
            modified = true;
            console.log(
              "✅ Fallback: inserted fragment as sole return argument.",
            );
          }
          // B) Si ya hay <div> o <form> en el return …
          else if (
            t.isJSXElement(arg) &&
            (t.isJSXIdentifier(arg.openingElement.name, { name: "div" }) ||
              t.isJSXIdentifier(arg.openingElement.name, { name: "form" }))
          ) {
            arg.children.push(t.cloneNode(fragmentAst, true));
            modified = true;
            console.log(
              `✅ Fallback: inserted fragment inside <${arg.openingElement.name.name}> of last return.`,
            );
          }
        }
      } else {
        let inserted = false;

        traverse(ast, {
          JSXElement(path) {
            const attr = path.node.openingElement.attributes.find(
              (a) => a.type === "JSXAttribute" && a.name.name === "data-id",
            );
            if (!attr || attr.value.value !== referenceId) return;
            console.log(
              "Procesando JSXElement para inserción, data-id:",
              attr.value.value,
            );

            switch (position) {
              case "before":
                path.insertBefore(t.cloneNode(fragmentAst, true));
                break;
              case "after":
                path.insertAfter(t.cloneNode(fragmentAst, true));
                break;
              case "inner":
                path.node.children.push(t.cloneNode(fragmentAst, true));
                break;
              default:
                console.error("❌ Unknown insert position:", position);
                return;
            }

            inserted = true;
            modified = true;
            path.stop();
          },
        });

        // Fallback: si no se encontró el referenceId
        if (!inserted) {
          console.warn(
            `! referenceId "${referenceId}" no encontrado. Aplicando fallback en el último return…`,
          );

          // 1) Encontrar el último ReturnStatement
          let lastReturnPath = null;
          traverse(ast, {
            ReturnStatement(path) {
              lastReturnPath = path;
            },
          });

          // 2) Si lo encontramos, aplicamos la lógica de fallback
          if (lastReturnPath) {
            const ret = lastReturnPath.node;
            const arg = ret.argument;

            // Caso A: return;  (sin argumento)
            if (arg == null) {
              // Lo convertimos en: return <...fragmentAst...>;
              ret.argument = t.cloneNode(fragmentAst, true);
              modified = true;
              console.log(
                "✅ Fragment insertado como único argumento del último return.",
              );
            }
            // Caso B: return <div>…</div>; o <form>…</form>;
            else if (
              t.isJSXElement(arg) &&
              (t.isJSXIdentifier(arg.openingElement.name, { name: "div" }) ||
                t.isJSXIdentifier(arg.openingElement.name, { name: "form" }))
            ) {
              // Lo añadimos como hijo
              arg.children.push(t.cloneNode(fragmentAst, true));
              modified = true;
              console.log(
                `✅ Fragment insertado como hijo de <${arg.openingElement.name.name}> en el último return.`,
              );
            }
            // Si el return ya era otro JSX distinto, aquí podrías decidir reemplazarlo o ignorar
          }
        }
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
