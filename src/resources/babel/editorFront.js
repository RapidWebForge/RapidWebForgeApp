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

      const modelImportExists = ast.program.body.some(
        (node) =>
          node.type === "ImportDeclaration" &&
          node.source.value === `../models/${model}`,
      );

      if (!modelImportExists)
        ast.program.body.unshift({
          type: "ImportDeclaration",
          specifiers: [
            {
              type: "ImportDefaultSpecifier",
              local: { type: "Identifier", name: model },
            },
          ],
          source: {
            type: "StringLiteral",
            value: `../models/${model}`,
          },
        });

      if (!serviceImportExists)
        ast.program.body.unshift({
          type: "ImportDeclaration",
          specifiers: [
            {
              type: "ImportDefaultSpecifier",
              local: { type: "Identifier", name: model + "Service" },
            },
          ],
          source: {
            type: "StringLiteral",
            value: `../services/${model}Service`,
          },
        });
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
            const oldModel = getAttrValue(path.node, "data-rwf-model");
            const newModel = getAttrValue(fragmentAst, "data-rwf-model");
            const oldMethod = getAttrValue(path.node, "data-rwf-method");
            const newMethod = getAttrValue(fragmentAst, "data-rwf-method");
            const isDiv = path.node.openingElement.name.name === "div";
            const isForm = path.node.openingElement.name.name === "form";

            const modelWasReplaced =
              oldModel && newModel && oldModel !== newModel;
            const modelWasRemoved = oldModel && !newModel;
            const modelWasAdded = newModel && !oldModel;

            const methodWasReplaced =
              oldMethod && newMethod && oldMethod !== newMethod;
            const methodWasRemoved = oldMethod && !newMethod;
            const methodWasAdded = newMethod && !oldMethod;

            if (isDiv) {
              // Eliminar useState y useEffect antiguos del oldModel
              if (modelWasReplaced || modelWasRemoved)
                traverse(ast, {
                  VariableDeclaration(path) {
                    const code = generate(path.node).code;
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
                    if (
                      code.includes("useEffect") &&
                      code.includes(`${oldModel}Service.getAll${oldModel}()`)
                    ) {
                      path.remove();
                      modified = true;
                    }
                  },
                });

              // Agregar nuevas definiciones
              if (modelWasReplaced || modelWasAdded) {
                const lowerNewModel = toLower(newModel);
                const stateCode = `const [${lowerNewModel}, set${newModel}] = useState<${newModel}[]>([]);`;
                const effectCode = `useEffect(() => {
                    ${newModel}Service.getAll${newModel}()
                    .then((response) => {
                      set${newModel}(response);
                    })
                    .catch((error) => {
                      console.error("Error fetching ${newModel} data:", error);
                    });
                  }, []);`;

                traverse(ast, {
                  FunctionDeclaration(path) {
                    if (path.node.id?.name === fileName) {
                      const stateNodeNew = template.ast(stateCode, {
                        plugins: ["jsx", "typescript"],
                      });
                      const effectNodeNew = template.ast(effectCode, {
                        plugins: ["jsx", "typescript"],
                      });
                      path.node.body.body.unshift(stateNodeNew);
                      path.node.body.body.unshift(effectNodeNew);
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
                const capitalizeOldMethod = toCapitalize(oldMethod);
                const lowerOldMethod = oldMethod.toLowerCase();

                traverse(ast, {
                  VariableDeclaration(path) {
                    const code = generate(path.node).code;
                    if (
                      code.includes(
                        `const [${lowerOldMethod}${oldModel}, set${capitalizeOldMethod}${oldModel}]`,
                      ) &&
                      code.includes(`useState<${oldModel}>()`)
                    ) {
                      path.remove();
                      modified = true;
                    }
                    if (
                      code.includes(
                        `const handleChange${capitalizeOldMethod}${oldModel}`,
                      ) &&
                      code.includes(`set${capitalizeOldMethod}${oldModel}`) &&
                      code.includes("[name]: value")
                    ) {
                      path.remove();
                      modified = true;
                    }
                    if (
                      code.includes(
                        `const handleSubmit${capitalizeOldMethod}${oldModel}`,
                      ) &&
                      code.includes(`${oldModel}Service.`)
                    ) {
                      path.remove();
                      modified = true;
                    }
                  },
                });
              }

              // Agregar nuevas definiciones
              if (
                modelWasReplaced ||
                modelWasAdded ||
                methodWasReplaced ||
                methodWasAdded
              ) {
                const capitalizeNewMethod = toCapitalize(newMethod);
                const lowerNewMethod = newMethod.toLowerCase();

                let methodService = null;

                if (newMethod === "PUT") methodService = "update";
                else if (newMethod === "POST") methodService = "create";

                const stateCode = `const [${lowerNewMethod}${newModel}, set${capitalizeNewMethod}${newModel}] = useState<${newModel}>();`;
                const handleChangeCode = `const handleChange = (e: any) => {
                   const { name, value } = e.target;
                   set${capitalizeNewMethod}${newModel}((prevData) => ({
                     ...prevData,
                     [name]: value,
                   }));
                 };`;
                const handleSubmitCode = `const handleSubmit = async (e: React.FormEvent) => {
                   e.preventDefault();
                   if (!${lowerNewMethod}${newModel}) {
                     console.error("Data is undefined");
                     return;
                   }
                   try {
                     const response = await ${newModel}Service.${methodService}${newModel}(${lowerNewMethod}${newModel});
                     console.log("Form submitted successfully:", response);
                   } catch (error) {
                     console.error("Error submitting form:", error);
                   }
                 };`;

                traverse(ast, {
                  FunctionDeclaration(path) {
                    if (path.node.id?.name === fileName) {
                      const stateNodeNew = template.ast(stateCode, {
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
              }
            }

            path.replaceWith(t.cloneNode(fragmentAst, true));

            if (modelWasReplaced || modelWasAdded || modelWasRemoved) {
              const newAst = parser.parse(generate(ast).code, {
                sourceType: "module",
                plugins: ["jsx", "typescript"],
              });

              let usesOldModel = false;
              let usesOldModelService = false;

              traverse(newAst, {
                Identifier(path) {
                  // Ignorar si viene de un import
                  if (path.findParent((p) => p.isImportDeclaration())) return;

                  if (path.node.name === oldModel) usesOldModel = true;
                  if (path.node.name === `${oldModel}Service`)
                    usesOldModelService = true;

                  // Si ya sabemos que se usan ambos, detenemos el análisis
                  if (usesOldModel && usesOldModelService) {
                    path.stop();
                  }
                },
              });

              let deletedOldModelImports = false;

              traverse(ast, {
                ImportDeclaration(importPath) {
                  const importSource = importPath.node.source.value;

                  const isModelImport =
                    importSource === `../models/${oldModel}`;
                  const isServiceImport =
                    importSource === `../services/${oldModel}Service`;

                  if (
                    (isModelImport && !usesOldModel) ||
                    (isServiceImport && !usesOldModelService)
                  ) {
                    importPath.remove();
                    deletedOldModelImports = true;
                    modified = true;
                  }
                },
              });

              // Solo después de eliminar, agrega si hace falta
              if (deletedOldModelImports) {
                checkMissingImports(newModel);
              }
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
                    code.includes(`useState<${model}>()`)
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

              let usesOldModel = false;
              let usesOldModelService = false;

              traverse(newAst, {
                Identifier(path) {
                  // Ignorar si viene de un import
                  if (path.findParent((p) => p.isImportDeclaration())) return;

                  if (path.node.name === model) usesOldModel = true;
                  if (path.node.name === `${model}Service`)
                    usesOldModelService = true;

                  // Si ya sabemos que se usan ambos, detenemos el análisis
                  if (usesOldModel && usesOldModelService) {
                    path.stop();
                  }
                },
              });

              traverse(ast, {
                ImportDeclaration(importPath) {
                  const importSource = importPath.node.source.value;

                  const isModelImport = importSource === `../models/${model}`;
                  const isServiceImport =
                    importSource === `../services/${model}Service`;

                  if (
                    (isModelImport && !usesOldModel) ||
                    (isServiceImport && !usesOldModelService)
                  ) {
                    importPath.remove();
                    modified = true;
                  }
                },
              });
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

      fileModified = true;
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

        const formStateCode = `const [${lowerMethod}${model}, set${capitalizeMethod}${model}] = useState<${model}>();`;
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
        traverse(ast, {
          JSXElement(path) {
            if (path.node.openingElement.name.name === "div" && !modified) {
              path.node.children.push(t.cloneNode(fragmentAst, true));
              modified = true;
              path.stop();
            }
          },
        });
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
            `! referenceId "${referenceId}" no encontrado. Verificando fallback en <div> raíz.`,
          );

          traverse(ast, {
            ReturnStatement(path) {
              const root = path.node.argument;
              if (
                root?.type === "JSXElement" &&
                root.openingElement.name.name === "div"
              ) {
                const childCount = root.children.filter(
                  (child) =>
                    child.type !== "JSXText" || child.value.trim() !== "",
                ).length;

                if (childCount <= 1) {
                  root.children.push(t.cloneNode(fragmentAst, true));
                  modified = true;
                  console.log(
                    "✅ Fragment insertado en <div> raíz como fallback.",
                  );
                } else {
                  console.warn(
                    "⛔ <div> raíz no está vacío. No se insertó como fallback.",
                  );
                }

                path.stop();
              }
            },
          });
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
