const fs = require("fs");
const { execSync } = require("child_process");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;
const t = require("@babel/types");
const template = require("@babel/template").default;

// --- justo después de los imports de babel y tus utilidades ---
const reorderComponentStatements = (ast, t, traverse) => {
  traverse(ast, {
    FunctionDeclaration(path) {
      const name = path.node.id?.name;
      if (!name || name[0] !== name[0].toUpperCase()) return;

      const groups = { router: [], state: [], effect: [], handler: [], other: [], render: [] };
      path.get('body').get('body').forEach(stmtPath => {
        const node = stmtPath.node;
        // 1) router hooks
        if (
          t.isVariableDeclaration(node) &&
          node.declarations[0].init?.callee?.name?.match(/^use(P|L|N)/)
        ) {
          groups.router.push(node);
          return;
        }
        // 2) useState
        if (
          t.isVariableDeclaration(node) &&
          node.declarations[0].init?.callee?.name === 'useState'
        ) {
          groups.state.push(node);
          return;
        }
        // 3) useEffect
        if (
          (t.isVariableDeclaration(node) &&
            node.declarations[0].init?.callee?.name === 'useEffect') ||
          (t.isExpressionStatement(node) &&
            node.expression.callee?.name === 'useEffect')
        ) {
          groups.effect.push(node);
          return;
        }
        // 4) handlers
        if (
          t.isVariableDeclaration(node) &&
          (t.isArrowFunctionExpression(node.declarations[0].init) ||
           t.isFunctionExpression(node.declarations[0].init))
        ) {
          groups.handler.push(node);
          return;
        }
        // 5) return
        if (t.isReturnStatement(node)) {
          groups.render.push(node);
          return;
        }
        // 6) resto
        groups.other.push(node);
      });

      // Reemplazo del body en orden
      const ordered = [
        ...groups.router,
        ...groups.state,
        ...groups.effect,
        ...groups.handler,
        ...groups.other,
        ...groups.render,
      ];
      path.get('body').node.body = ordered.map(n => t.cloneDeep(n));
    }
  });
};

// Custom error handling for babel traverse
const safeTraverse = (ast, visitor) => {
  try {
    traverse(ast, visitor);
  } catch (error) {
    console.error("❌ Error during AST traversal:", error.message);
    return false;
  }
  return true;
};

const [, , filePath, operation, referenceId, ...rest] = process.argv;

const getAttrValue = (node, attrName) => {
  const attr = node.openingElement.attributes.find(
    (a) => a.type === "JSXAttribute" && a.name.name === attrName
  );
  if (!attr || !attr.value) return null;

  if (attr.value.type === "StringLiteral") {
    return attr.value.value;
  }

  if (attr.value.type === "JSXExpressionContainer") {
    return generate(attr.value.expression).code;
  }

  return null;
};

const toLower = (s) => s.charAt(0).toLowerCase() + s.slice(1);

const toCapitalize = (str) => {
  if (!str) return "";
  return str.charAt(0).toUpperCase() + str.slice(1).toLowerCase();
};

const extractObjectName = (code) => {
  if (!code || typeof code !== "string") return null;

  // Extrae todo lo que esté dentro de deleteXXXById(...)
  const match = code.match(/delete([A-Za-z0-9]+)ById\s*\(/);
  if (match?.[1]) {
    return match[1];
  }

  return null;
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
      // 1) ¿Ya hay import del servicio?
      const serviceImportExists = ast.program.body.some(
        (node) =>
          node.type === "ImportDeclaration" &&
          node.source.value === `../services/${model}Service`
      );

      // 2) ¿Ya hay import de modelos?
      const importDeclIndex = ast.program.body.findIndex(
        (node) =>
          t.isImportDeclaration(node) &&
          node.source.value === `../models/${model}`
      );

      if (importDeclIndex !== -1) {
        const importDecl = ast.program.body[importDeclIndex];

        // -- Asegurar default import para el modelo
        const hasDefault = importDecl.specifiers.some(
          t.isImportDefaultSpecifier
        );
        if (!hasDefault) {
          // eliminar viejo named import si existiera
          importDecl.specifiers = importDecl.specifiers.filter(
            (spec) =>
              !(t.isImportSpecifier(spec) && spec.imported.name === model)
          );
          importDecl.specifiers.unshift(
            t.importDefaultSpecifier(t.identifier(model))
          );
        }

        // -- Asegurar named import para Defaults
        const defaultsName = `${model}Defaults`;
        const hasDefaults = importDecl.specifiers.some(
          (spec) =>
            t.isImportSpecifier(spec) && spec.imported.name === defaultsName
        );
        if (!hasDefaults) {
          importDecl.specifiers.push(
            t.importSpecifier(
              t.identifier(defaultsName),
              t.identifier(defaultsName)
            )
          );
        }
      } else {
        // 3) Si no existía import del modelo, lo creamos de cero
        const newImport = t.importDeclaration(
          [
            t.importDefaultSpecifier(t.identifier(model)),
            t.importSpecifier(
              t.identifier(`${model}Defaults`),
              t.identifier(`${model}Defaults`)
            ),
          ],
          t.stringLiteral(`../models/${model}`)
        );
        ast.program.body.unshift(newImport);
      }

      // 4) Y finalmente, el import del servicio como default
      if (!serviceImportExists) {
        const svcImport = t.importDeclaration(
          [t.importDefaultSpecifier(t.identifier(`${model}Service`))],
          t.stringLiteral(`../services/${model}Service`)
        );
        ast.program.body.unshift(svcImport);
      }
    };

    const ensureReactHooksImport = (ast, hooks = []) => {
      if (!ast || !ast.program || !ast.program.body) {
        console.error("❌ Invalid AST structure in ensureReactHooksImport");
        return;
      }

      const reactImport = ast.program.body.find(
        (node) =>
          node.type === "ImportDeclaration" && node.source.value === "react"
      );

      if (reactImport) {
        const existingSpecifiers = new Set(
          reactImport.specifiers.map((s) =>
            s.type === "ImportSpecifier" ? s.imported.name : null
          )
        );

        const missingHooks = hooks.filter(
          (hook) => !existingSpecifiers.has(hook)
        );

        // Agregar los hooks faltantes como importaciones con nombre
        if (missingHooks.length > 0) {
          reactImport.specifiers.push(
            ...missingHooks.map((hook) => ({
              type: "ImportSpecifier",
              imported: { type: "Identifier", name: hook },
              local: { type: "Identifier", name: hook },
            }))
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
        fragmentAst = parser.parseExpression(raw, {
          plugins: ["jsx"],
        });
      } catch (e) {
        console.error("❌ Error parsing payload:", e.message);
        fragmentAst = null;
      }

    if (operation === "modify" || operation === "delete") {
      safeTraverse(ast, {
        JSXElement(path) {
          try {
            if (
              !path ||
              !path.node ||
              !path.node.openingElement ||
              !path.node.openingElement.attributes
            )
              return;

            const attr = path.node.openingElement.attributes.find(
              (a) => a.type === "JSXAttribute" && a.name.name === "data-id"
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
              const isButton = path.node.openingElement.name.name === "button";

              if (isButton) {
                const oldClickRaw = getAttrValue(path.node, "onClick");
                const newClickRaw = getAttrValue(fragmentAst, "onClick");

                const oldClick = oldClickRaw?.trim() ? oldClickRaw : null;
                const newClick = newClickRaw?.trim() ? newClickRaw : null;

                const hasOldClick = oldClick !== null;
                const hasNewClick = newClick !== null;

                const clickWasReplaced =
                  hasOldClick && hasNewClick && oldClick !== newClick;
                const clickWasRemoved = hasOldClick && !hasNewClick;
                const clickWasAdded = hasNewClick && !hasOldClick;

                if (clickWasReplaced || clickWasRemoved) {
                  let buttonModelFound = false;
                  let buttonModel;

                  if (hasOldClick) {
                    let model = null;
                    model = extractObjectName(oldClick);
                    if (model) {
                      traverse(ast, {
                        VariableDeclaration(path) {
                          const code = generate(path.node).code;
                          if (
                            code.includes(`const delete${model}ById`) &&
                            code.includes(`${model}Service.`)
                          ) {
                            path.remove();
                            modified = true;
                          }
                        },
                      });
                      buttonModelFound = true;
                      buttonModel = model;
                    }
                  }

                  if (buttonModelFound) {
                    const newAst = parser.parse(generate(ast).code, {
                      sourceType: "module",
                      plugins: ["jsx", "typescript"],
                    });

                    const defaultsButton = `${buttonModel}Defaults`;
                    let useButtonModel = false;
                    let usesButtonModelDefaults = false;
                    let usesButtonModelService = false;

                    safeTraverse(newAst, {
                      Identifier(path) {
                        if (path.findParent((p) => p.isImportDeclaration()))
                          return;
                        const name = path.node.name;
                        if (name === buttonModel) useButtonModel = true;
                        if (name === defaultsButton)
                          usesButtonModelDefaults = true;
                        if (name === `${buttonModel}Service`)
                          usesButtonModelService = true;
                        if (
                          useButtonModel &&
                          usesButtonModelDefaults &&
                          usesButtonModelService
                        ) {
                          path.stop();
                        }
                      },
                    });

                    safeTraverse(ast, {
                      ImportDeclaration(path) {
                        const src = path.node.source.value;

                        // 2.1) Servicio
                        if (
                          src === `../services/${buttonModel}Service` &&
                          !usesButtonModelService
                        ) {
                          path.remove();
                          modified = true;
                          return;
                        }

                        // 2.2) Modelo + defaults
                        if (src === `../models/${buttonModel}`) {
                          let changed = false;

                          path.node.specifiers = path.node.specifiers.filter(
                            (spec) => {
                              // import Model, {defaults } ...
                              if (t.isImportSpecifier(spec)) {
                                const name = spec.imported.name;
                                if (name === buttonModel && !useButtonModel) {
                                  changed = true;
                                  return false;
                                }
                                if (
                                  name === defaultsButton &&
                                  !usesButtonModelDefaults
                                ) {
                                  changed = true;
                                  return false;
                                }
                              }
                              // import Tasks from ...
                              if (
                                t.isImportDefaultSpecifier(spec) &&
                                spec.local.name === buttonModel &&
                                !useButtonModel
                              ) {
                                changed = true;
                                return false;
                              }
                              return true;
                            }
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
                  }
                }

                if (clickWasReplaced || clickWasAdded) {
                  if (hasNewClick) {
                    const model = extractObjectName(newClick);
                    if (model) {
                      checkMissingImports(model);

                      const code = `const delete${model}ById = async (id: number) => {
                        try {
                          const response = await ${model}Service.delete${model}ById(id);
                          console.log("Element deleted successfully:", response);
                        } catch (error) {
                          console.error("Error deleting element:", error);
                        }
                      };`;

                      safeTraverse(ast, {
                        FunctionDeclaration(path) {
                          if (!path || !path.node || !path.node.id) return;
                          if (path.node.id?.name === fileName) {
                            const node = template.ast(code, {
                              plugins: ["jsx", "typescript"],
                            });
                            path.node.body.body.unshift(node);
                          }
                        },
                      });
                    }
                  }
                }
              }
              if (isDiv) {
                const oldGetRaw = getAttrValue(path.node, "data-rwf-get");
                const newGetRaw = getAttrValue(fragmentAst, "data-rwf-get");

                const oldGet = oldGetRaw?.trim() ? oldGetRaw : null;
                const newGet = newGetRaw?.trim() ? newGetRaw : null;

                const hasOldGet = oldGet !== null;
                const hasNewGet = newGet !== null;

                const getWasReplaced =
                  hasOldGet && hasNewGet && oldGet !== newGet;
                const getWasRemoved = hasOldGet && !hasNewGet;
                const getWasAdded = hasNewGet && !hasOldGet;

                // Eliminar useState y useEffect antiguos del oldModel
                if (
                  ((modelWasReplaced || modelWasRemoved) && hasOldModel) ||
                  ((getWasReplaced || getWasRemoved) && hasOldGet)
                ) {
                  safeTraverse(ast, {
                    VariableDeclaration(path) {
                      const code = generate(path.node).code;
                      if (oldGet === "ALL")
                        if (
                          code.includes(
                            `const [${toLower(oldModel)}, set${oldModel}]`
                          ) &&
                          code.includes(`useState<${oldModel}[]>`)
                        ) {
                          path.remove();
                          modified = true;
                        }
                      if (oldGet === "ID")
                        if (
                          code.includes(
                            `const [${toLower(oldModel)}, set${oldModel}]`
                          ) &&
                          code.includes(`useState<${oldModel}>`)
                        ) {
                          path.remove();
                          modified = true;
                        }
                    },
                    ExpressionStatement(path) {
                      const code = generate(path.node).code;
                      if (oldGet === "ALL")
                        if (
                          code.includes("useEffect") &&
                          code.includes(
                            `${oldModel}Service.getAll${oldModel}()`
                          )
                        ) {
                          path.remove();
                          modified = true;
                        }
                      if (oldGet === "ID")
                        if (
                          code.includes("useEffect") &&
                          code.includes(
                            `${oldModel}Service.get${oldModel}ById(${toLower(
                              oldModel
                            )}Id)`
                          )
                        ) {
                          path.remove();
                          modified = true;
                        }
                    },
                  });
                }

                // Agregar nuevas definiciones
                if (
                  ((modelWasReplaced || modelWasAdded) &&
                    hasNewModel &&
                    (getWasReplaced || getWasAdded) &&
                    hasNewGet) ||
                  ((getWasReplaced || getWasAdded) &&
                    hasNewGet &&
                    hasNewModel) ||
                  ((modelWasReplaced || modelWasAdded) &&
                    hasNewModel &&
                    hasNewGet)
                ) {
                  const lowerNewModel = toLower(newModel);
                  const modelParam = `${lowerNewModel}Id`;
                  let modelParamFound = false;

                  safeTraverse(ast, {
                    VariableDeclarator(path) {
                      if (!path || !path.node) return;
                      // buscamos ObjectPattern = useParams()
                      if (
                        t.isObjectPattern(path.node.id) &&
                        t.isCallExpression(path.node.init) &&
                        t.isIdentifier(path.node.init.callee, {
                          name: "useParams",
                        })
                      ) {
                        // ¿está nuestro parámetro entre las propiedades?
                        const hasParam = path.node.id.properties.some(
                          (prop) =>
                            t.isObjectProperty(prop) &&
                            t.isIdentifier(prop.key, { name: modelParam })
                        );
                        if (hasParam) {
                          modelParamFound = true;
                        }
                        path.stop();
                      }
                    },
                  });

                  if (
                    (newGet === "ID" && modelParamFound) ||
                    newGet === "ALL"
                  ) {
                    let stateCode;
                    if (newGet === "ALL") {
                      stateCode = `const [${lowerNewModel}, set${newModel}] = useState<${newModel}[]>([]);`;
                    } else if (newGet === "ID") {
                      stateCode = `const [${lowerNewModel}, set${newModel}] = useState<${newModel}>();`;
                    }
                    let effectCode;
                    if (newGet === "ALL") {
                      effectCode = `
                        useEffect(() => {
                          ${newModel}Service.getAll${newModel}()
                            .then(response => set${newModel}(response))
                            .catch(error => console.error("Error fetching ${newModel} data:", error));
                        }, []);
                      `.trim();
                    } else if (newGet === "ID") {
                      effectCode = `
                        useEffect(() => {
                          ${newModel}Service.get${newModel}ById(${modelParam})
                            .then(response => set${newModel}(response))
                            .catch(error => console.error("Error fetching ${newModel} by id:", error));
                        }, [${modelParam}]);
                      `.trim();
                    }
                    safeTraverse(ast, {
                      FunctionDeclaration(path) {
                        if (!path || !path.node || !path.node.id) return;
                        if (path.node.id?.name === fileName) {
                          // Generar nodos con statement.ast
                          const stateNode = template.ast(stateCode, {
                            plugins: ["jsx", "typescript"],
                          });
                          console.log("stateNode", stateNode.type);
                          const effectNode = template.ast(effectCode, {
                            plugins: ["jsx", "typescript"],
                          });
                          // Insertar en el orden que prefieras
                          path.node.body.body.unshift(stateNode);
                          path.node.body.body.unshift(effectNode);
                        }
                      },
                    });
                    ensureReactHooksImport(ast, ["useState", "useEffect"]);
                  }
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

                    if (oldMethod === "PUT") {
                      safeTraverse(ast, {
                        ExpressionStatement(path) {
                          const expr = path.node.expression;

                          if (
                            t.isCallExpression(expr) &&
                            t.isIdentifier(expr.callee, {
                              name: "useEffect",
                            }) &&
                            expr.arguments.length === 2 &&
                            t.isArrayExpression(expr.arguments[1])
                          ) {
                            const deps = expr.arguments[1].elements;

                            const modelIdName = `${toLower(oldModel)}Id`;

                            const hasTargetDep = deps.some(
                              (el) =>
                                t.isIdentifier(el) && el.name === modelIdName
                            );

                            if (hasTargetDep) {
                              path.remove();
                              modified = true;
                            }
                          }
                        },
                      });
                    }

                    safeTraverse(ast, {
                      VariableDeclaration(path) {
                        const code = generate(path.node).code;

                        // Remover useState
                        if (
                          code.includes(
                            `const [${lowOldMtd}${oldModel}, set${capOldMtd}${oldModel}]`
                          ) &&
                          code.includes(
                            `useState<${oldModel}>(${oldModel}Defaults.default${capOldMtd}${oldModel})`
                          )
                        ) {
                          path.remove();
                          modified = true;
                        }

                        // Remover handleChange
                        if (
                          code.includes(
                            `const handleChange${capOldMtd}${oldModel}`
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
                            `const handleSubmit${capOldMtd}${oldModel}`
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

                    const lowNewModel = toLower(newModel);
                    const modelParam = `${lowNewModel}Id`;

                    let effectCodeUpdate = null;
                    if (newMethod === "PUT") {
                      effectCodeUpdate = `useEffect(() => {
                    ${newModel}Service.get${newModel}ById(${modelParam})
                      .then((response) => {
                          set${capNewMtd}${newModel}(response);
                      })
                      .catch((error) => {
                        console.error("Error fetching ${newModel} data by id:", error);
                      });
                    }, [${modelParam}]);`;
                    }

                    let modelParamFound = false;

                    safeTraverse(ast, {
                      VariableDeclarator(path) {
                        // buscamos ObjectPattern = useParams()
                        if (
                          t.isObjectPattern(path.node.id) &&
                          t.isCallExpression(path.node.init) &&
                          t.isIdentifier(path.node.init.callee, {
                            name: "useParams",
                          })
                        ) {
                          // ¿está nuestro parámetro entre las propiedades?
                          const hasParam = path.node.id.properties.some(
                            (prop) =>
                              t.isObjectProperty(prop) &&
                              t.isIdentifier(prop.key, { name: modelParam })
                          );
                          if (hasParam) {
                            modelParamFound = true;
                          }
                          path.stop();
                        }
                      },
                    });

                    const stateCode = `
                    const [${lowNewMtd}${newModel}, set${capNewMtd}${newModel}] = useState<${newModel}>(${newModel}Defaults.default${capNewMtd}${newModel});
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

                    let handleSubmitCode;
                    if (newMethod === "PUT")
                      handleSubmitCode = `
                      const handleSubmit${capNewMtd}${newModel} = async (e: React.FormEvent) => {
                        e.preventDefault();
                        if (!${lowNewMtd}${newModel}) {
                          console.error("Data is undefined");
                          return;
                        }
                        try {
                          const response = await ${newModel}Service.${methodService}${newModel}ById(${lowNewMtd}${newModel}.id, ${lowNewMtd}${newModel});
                          console.log("Form submitted successfully:", response);
                        } catch (error) {
                          console.error("Error submitting form:", error);
                        }
                      };
                      `.trim();
                    if (newMethod === "POST")
                      handleSubmitCode = `
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

                    safeTraverse(ast, {
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
                          let effectNode = null;
                          if (modelParamFound && effectCodeUpdate)
                            effectNode = template.ast(effectCodeUpdate, {
                              plugins: ["jsx", "typescript"],
                            });

                          // Insertamos en orden: state, change, submit
                          path.node.body.body.unshift(submitNode);
                          path.node.body.body.unshift(changeNode);
                          path.node.body.body.unshift(stateNode);
                          if (effectNode)
                            path.node.body.body.unshift(effectNode);
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

                const defaults = `${oldModel}Defaults`;
                let usesOldModel = false;
                let usesDefaultModel = false;
                let usesOldModelService = false;

                // 1) Detectar si siguen usándose en el código
                safeTraverse(newAst, {
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
                safeTraverse(ast, {
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
                          // Model, { ModelDefaults }
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
                        }
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
            }
            if (operation === "delete") {
              let deletedComponentName = null;

              if (path.node.openingElement.name.type === "JSXIdentifier") {
                deletedComponentName = path.node.openingElement.name.name;
              }

              const model = getAttrValue(path.node, "data-rwf-model");
              const method = getAttrValue(path.node, "data-rwf-method");
              const isDiv = path.node.openingElement.name.name === "div";
              const isForm = path.node.openingElement.name.name === "form";
              const isButton = path.node.openingElement.name.name === "button";
              let buttonModelFound = false;
              let buttonModel;

              if (isButton) {
                const onClick = getAttrValue(path.node, "onClick");
                let model = null;
                if (onClick) model = extractObjectName(onClick);
                if (model) {
                  traverse(ast, {
                    VariableDeclaration(path) {
                      const code = generate(path.node).code;
                      if (
                        code.includes(`const delete${model}ById`) &&
                        code.includes(`${model}Service.`)
                      ) {
                        path.remove();
                        modified = true;
                      }
                    },
                  });
                  buttonModelFound = true;
                  buttonModel = model;
                }
              }
              if (isDiv && model) {
                const get = getAttrValue(path.node, "data-rwf-get");

                traverse(ast, {
                  VariableDeclaration(path) {
                    const code = generate(path.node).code;
                    if (get === "ALL")
                      if (
                        code.includes(
                          `const [${toLower(model)}, set${model}]`
                        ) &&
                        code.includes(`useState<${model}[]>`)
                      ) {
                        path.remove();
                        modified = true;
                      }
                    if (get === "ID")
                      if (
                        code.includes(
                          `const [${toLower(model)}, set${model}]`
                        ) &&
                        code.includes(`useState<${model}>`)
                      ) {
                        path.remove();
                        modified = true;
                      }
                  },
                  ExpressionStatement(path) {
                    const code = generate(path.node).code;
                    if (get === "ALL")
                      if (
                        code.includes("useEffect") &&
                        code.includes(`${model}Service.getAll${model}()`)
                      ) {
                        path.remove();
                        modified = true;
                      }
                    if (get === "ID")
                      if (
                        code.includes("useEffect") &&
                        code.includes(
                          `${model}Service.get${model}ById(${toLower(model)}Id)`
                        )
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
                  ExpressionStatement(path) {
                    const expr = path.node.expression;

                    if (
                      t.isCallExpression(expr) &&
                      t.isIdentifier(expr.callee, { name: "useEffect" }) &&
                      expr.arguments.length === 2 &&
                      t.isArrayExpression(expr.arguments[1])
                    ) {
                      const deps = expr.arguments[1].elements;

                      const modelIdName = `${toLower(model)}Id`;

                      const hasTargetDep = deps.some(
                        (el) => t.isIdentifier(el) && el.name === modelIdName
                      );

                      if (hasTargetDep) {
                        path.remove();
                        modified = true;
                      }
                    }
                  },
                  VariableDeclaration(path) {
                    const code = generate(path.node).code;
                    if (
                      code.includes(
                        `const [${lowerMethod}${model}, set${capitalizeMethod}${model}]`
                      ) &&
                      code.includes(
                        `useState<${model}>(${model}Defaults.default${capitalizeMethod}${model})`
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

                const defaults = `${model}Defaults`;
                let usesOldModel = false;
                let usesDefaultModel = false;
                let usesOldModelService = false;

                // 1) Detectar usos en newAst
                safeTraverse(newAst, {
                  Identifier(path) {
                    if (path.findParent((p) => p.isImportDeclaration())) return;
                    const name = path.node.name;
                    if (name === model) usesOldModel = true;
                    if (name === defaults) usesDefaultModel = true;
                    if (name === `${model}Service`) usesOldModelService = true;
                    if (
                      usesOldModel &&
                      usesDefaultModel &&
                      usesOldModelService
                    ) {
                      path.stop();
                    }
                  },
                });

                // 2) Limpiar imports obsoletos en ast
                safeTraverse(ast, {
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
                        }
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
              }

              if (isButton && buttonModelFound) {
                const newAst = parser.parse(generate(ast).code, {
                  sourceType: "module",
                  plugins: ["jsx", "typescript"],
                });

                const defaultsButton = `${buttonModel}Defaults`;
                let useButtonModel = false;
                let usesButtonModelDefaults = false;
                let usesButtonModelService = false;

                safeTraverse(newAst, {
                  Identifier(path) {
                    if (path.findParent((p) => p.isImportDeclaration())) return;
                    const name = path.node.name;
                    if (name === buttonModel) useButtonModel = true;
                    if (name === defaultsButton) usesButtonModelDefaults = true;
                    if (name === `${buttonModel}Service`)
                      usesButtonModelService = true;
                    if (
                      useButtonModel &&
                      usesButtonModelDefaults &&
                      usesButtonModelService
                    ) {
                      path.stop();
                    }
                  },
                });

                safeTraverse(ast, {
                  ImportDeclaration(path) {
                    const src = path.node.source.value;

                    // 2.1) Servicio
                    if (
                      src === `../services/${buttonModel}Service` &&
                      !usesButtonModelService
                    ) {
                      path.remove();
                      modified = true;
                      return;
                    }

                    // 2.2) Modelo + defaults
                    if (src === `../models/${buttonModel}`) {
                      let changed = false;

                      path.node.specifiers = path.node.specifiers.filter(
                        (spec) => {
                          // import Model, {defaults } ...
                          if (t.isImportSpecifier(spec)) {
                            const name = spec.imported.name;
                            if (name === buttonModel && !useButtonModel) {
                              changed = true;
                              return false;
                            }
                            if (
                              name === defaultsButton &&
                              !usesButtonModelDefaults
                            ) {
                              changed = true;
                              return false;
                            }
                          }
                          // import Tasks from ...
                          if (
                            t.isImportDefaultSpecifier(spec) &&
                            spec.local.name === buttonModel &&
                            !useButtonModel
                          ) {
                            changed = true;
                            return false;
                          }
                          return true;
                        }
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
              }

              // Ahora, si era un custom component, verificamos si quedan instancias
              if (deletedComponentName) {
                let found = false;

                traverse(ast, {
                  JSXElement(path) {
                    if (
                      path.node.openingElement.name.type === "JSXIdentifier" &&
                      path.node.openingElement.name.name ===
                        deletedComponentName
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
                          spec.local.name === deletedComponentName
                      );

                      if (
                        importPath.includes(
                          `../components/${deletedComponentName}`
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
            if (path && typeof path.stop === "function") {
              path.stop();
            }
          } catch (e) {
            console.error("❌ Error processing JSX element:", e.message);
          }
        },
      });
    }

    if (operation === "refactor-delete" && referenceId) {
      const importName = referenceId;

      if (position === "View") {
        // Refactor para vistas (views)
        safeTraverse(ast, {
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
                    p.isVariableDeclaration()
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
                    importName
              )
            ) {
              path.remove();
              modified = true;
            }
          },
        });
      } else if (position === "CustomComponent") {
        // Refactor para componentes personalizados
        safeTraverse(ast, {
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
          if (!path || !path.node || !path.node.body) {
            console.error("❌ Invalid Program path structure");
            return;
          }
          // encuentra el índice donde terminan los import
          let lastImport = 0;
          path.node.body.forEach((n, i) => {
            if (t.isImportDeclaration(n)) lastImport = i;
          });
          const importNode = template.statement.ast(
            `const ${viewName} = React.lazy(() => import("./views/${viewName}"));`
          );
          path.node.body.splice(lastImport + 1, 0, importNode);
          path.stop();
        },
      });

      // 2) Insertar <Route> dentro de <Routes>
      safeTraverse(ast, {
        JSXElement(path) {
          if (!path || !path.node || !path.node.openingElement) return;
          // Detecta <Routes> ... </Routes>
          if (
            t.isJSXIdentifier(path.node.openingElement.name, { name: "Routes" })
          ) {
            // Crear el nuevo nodo <Route path="..." element={<View />} />
            const routeNode = parser.parseExpression(
              `<Route path="${routePath}" element={<${viewName} />} />`,
              { plugins: ["jsx"] }
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
                  spec.local.name === insertedComponentName
              )
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

      // Button with click
      if (insertedComponentName === "button") {
        const click = getAttrValue(fragmentAst, "onClick");

        if (click) {
          const model = extractObjectName(click);
          if (model) {
            checkMissingImports(model);

            const code = `const delete${model}ById = async (id: number) => {
              try {
                const response = await ${model}Service.delete${model}ById(id);
                console.log("Element deleted successfully:", response);
              } catch (error) {
                console.error("Error deleting element:", error);
              }
            };`;

            safeTraverse(ast, {
              FunctionDeclaration(path) {
                if (!path || !path.node || !path.node.id) return;
                if (path.node.id?.name === fileName) {
                  const node = template.ast(code, {
                    plugins: ["jsx", "typescript"],
                  });
                  path.node.body.body.unshift(node);
                }
              },
            });
          }
        }
      }

      // Model Layout
      if (model && insertedComponentName === "div") {
        checkMissingImports(model);

        const lowerModel = toLower(model);
        const modelParam = `${lowerModel}Id`;
        const get = getAttrValue(fragmentAst, "data-rwf-get");
        let modelParamFound = false;

        safeTraverse(ast, {
          VariableDeclarator(path) {
            if (!path || !path.node) return;
            // buscamos ObjectPattern = useParams()
            if (
              t.isObjectPattern(path.node.id) &&
              t.isCallExpression(path.node.init) &&
              t.isIdentifier(path.node.init.callee, { name: "useParams" })
            ) {
              // ¿está nuestro parámetro entre las propiedades?
              const hasParam = path.node.id.properties.some(
                (prop) =>
                  t.isObjectProperty(prop) &&
                  t.isIdentifier(prop.key, { name: modelParam })
              );
              if (hasParam) {
                modelParamFound = true;
              }
              path.stop();
            }
          },
        });

        if ((get === "ID" && modelParamFound) || get === "ALL") {
          let stateCode;
          if (get === "ALL") {
            stateCode = `const [${lowerModel}, set${model}] = useState<${model}[]>([]);`;
          } else if (get === "ID") {
            stateCode = `const [${lowerModel}, set${model}] = useState<${model}>();`;
          }
          let effectCode;
          if (get === "ALL") {
            effectCode = `
            useEffect(() => {
              ${model}Service.getAll${model}()
                .then(response => set${model}(response))
                .catch(error => console.error("Error fetching ${model} data:", error));
            }, []);
          `.trim();
          } else if (get === "ID") {
            effectCode = `
            useEffect(() => {
              ${model}Service.get${model}ById(${modelParam})
                .then(response => set${model}(response))
                .catch(error => console.error("Error fetching ${model} by id:", error));
            }, [${modelParam}]);
          `.trim();
          }
          safeTraverse(ast, {
            FunctionDeclaration(path) {
              if (!path || !path.node || !path.node.id) return;
              if (path.node.id?.name === fileName) {
                // Generar nodos con statement.ast
                const stateNode = template.ast(stateCode, {
                  plugins: ["jsx", "typescript"],
                });
                console.log("stateNode", stateNode.type);
                const effectNode = template.ast(effectCode, {
                  plugins: ["jsx", "typescript"],
                });
                // Insertar en el orden que prefieras
                path.node.body.body.unshift(stateNode);
                path.node.body.body.unshift(effectNode);
              }
            },
          });
          ensureReactHooksImport(ast, ["useState", "useEffect"]);
        }
        modified = true;
      }

      // Form
      if (model && insertedComponentName === "form") {
        checkMissingImports(model);
        ensureReactHooksImport(ast, ["useState", "useEffect"]);
      }
      if (method && insertedComponentName === "form") {
        const lowerMethod = method.toLowerCase();
        const lowerModel = toLower(model);
        const modelParam = `${lowerModel}Id`;
        const capitalizeMethod = toCapitalize(method);

        let methodService = null;

        if (method === "PUT") methodService = "update";
        else if (method === "POST") methodService = "create";

        let effectCodeUpdate;
        if (method === "PUT") {
          effectCodeUpdate = `useEffect(() => {
            ${model}Service.get${model}ById(${modelParam})
              .then((response) => {
                  set${capitalizeMethod}${model}(response);
              })
              .catch((error) => {
                console.error("Error fetching ${model} data by id:", error);
              });
          }, [${modelParam}]);`;
        }

        let modelParamFound = false;

        safeTraverse(ast, {
          VariableDeclarator(path) {
            if (!path || !path.node) return;
            // buscamos ObjectPattern = useParams()
            if (
              t.isObjectPattern(path.node.id) &&
              t.isCallExpression(path.node.init) &&
              t.isIdentifier(path.node.init.callee, { name: "useParams" })
            ) {
              // ¿está nuestro parámetro entre las propiedades?
              const hasParam = path.node.id.properties.some(
                (prop) =>
                  t.isObjectProperty(prop) &&
                  t.isIdentifier(prop.key, { name: modelParam })
              );
              if (hasParam) {
                modelParamFound = true;
              }
              path.stop();
            }
          },
        });

        const formStateCode = `const [${lowerMethod}${model}, set${capitalizeMethod}${model}] = useState<${model}>(${model}Defaults.default${capitalizeMethod}${model});`;
        const handleChangeCode = `const handleChange${capitalizeMethod}${model} = (e: any) => {
                 const { name, value } = e.target;
                 set${capitalizeMethod}${model}((prevData) => ({
                   ...prevData,
                   [name]: value,
                 }));
               };`;
        let handleSubmitCode;
        if (method === "PUT")
          handleSubmitCode = `const handleSubmit${capitalizeMethod}${model} = async (e: React.FormEvent) => {
                 e.preventDefault();
                 if (!${lowerMethod}${model}) {
                   console.error("Data is undefined");
                   return;
                 }
                 try {
                   const response = await ${model}Service.${methodService}${model}ById(${lowerMethod}${model}.id, ${lowerMethod}${model});
                   console.log("Form submitted successfully:", response);
                 } catch (error) {
                   console.error("Error submitting form:", error);
                 }
               };`;
        if (method === "POST")
          handleSubmitCode = `const handleSubmit${capitalizeMethod}${model} = async (e: React.FormEvent) => {
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

        safeTraverse(ast, {
          FunctionDeclaration(path) {
            if (!path || !path.node || !path.node.id) return;
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
              let effectNodeNew = null;
              if (modelParamFound && effectCodeUpdate) {
                effectNodeNew = template.ast(effectCodeUpdate, {
                  plugins: ["jsx", "typescript"],
                });
              }
              path.node.body.body.unshift(stateNodeNew);
              path.node.body.body.unshift(changeNodeNew);
              path.node.body.body.unshift(submitNodeNew);
              if (effectNodeNew) path.node.body.body.unshift(effectNodeNew);
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
        safeTraverse(ast, {
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
              "✅ Fallback: inserted fragment as sole return argument."
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
              `✅ Fallback: inserted fragment inside <${arg.openingElement.name.name}> of last return.`
            );
          }
        }
      } else {
        let inserted = false;

        safeTraverse(ast, {
          JSXElement(path) {
            try {
              if (
                !path ||
                !path.node ||
                !path.node.openingElement ||
                !path.node.openingElement.attributes
              )
                return;
              const attr = path.node.openingElement.attributes.find(
                (a) => a.type === "JSXAttribute" && a.name.name === "data-id"
              );
              if (!attr || attr.value.value !== referenceId) return;
              console.log(
                "Procesando JSXElement para inserción, data-id:",
                attr.value.value
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
              if (path && typeof path.stop === "function") {
                path.stop();
              }
            } catch (e) {
              console.error("❌ Error during JSXElement insertion:", e.message);
            }
          },
        });

        // Fallback: si no se encontró el referenceId
        if (!inserted) {
          console.warn(
            `! referenceId "${referenceId}" no encontrado. Aplicando fallback en el último return…`
          );

          // 1) Encontrar el último ReturnStatement
          let lastReturnPath = null;
          safeTraverse(ast, {
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
                "✅ Fragment insertado como único argumento del último return."
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
                `✅ Fragment insertado como hijo de <${arg.openingElement.name.name}> en el último return.`
              );
            }
            // Si el return ya era otro JSX distinto, aquí podrías decidir reemplazarlo o ignorar
          }
        }
      }
    }

    try {
      reorderComponentStatements(ast, t, traverse);

      const output = generate(ast, { retainLines: true }, sourceCode);

      // Guardar el código antes de formatear
      fs.writeFileSync(filePath, output.code);

      // Formatear con Biome (requiere que esté en node_modules/.bin o accesible desde bun)
      process.env.PATH = `${process.cwd()}/node_modules/.bin:${
        process.env.PATH
      }`;
      execSync(`biome format ${filePath} --write`, { stdio: "inherit" });

      console.log("✅ Code successfully modified and formatted.");
    } catch (genError) {
      console.error("❌ Error generating code:", genError.message);
      process.exit(1);
    }
  } catch (err) {
    console.error("❌ Unexpected error:", err.message);
    process.exit(1);
  }
})();
