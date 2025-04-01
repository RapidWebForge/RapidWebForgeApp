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
            path.replaceWith(t.cloneNode(fragmentAst, true));
          } else if (operation === "delete") {
            let deletedComponentName = null;

            if (path.node.openingElement.name.type === "JSXIdentifier") {
              deletedComponentName = path.node.openingElement.name.name;
            }

            path.remove();
            modified = true;

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

      const fileName = filePath.split("/").pop().split(".")[0];

      const checkMissingImports = () => {
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

      if (model && !method && insertedComponentName === "div") {
        checkMissingImports();

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

        let stateNode = null;
        let effectNode = null;

        try {
          stateNode = template.ast(stateCode, {
            plugins: ["jsx", "typescript"],
          });
        } catch (e) {
          console.error(
            "❌ Error generando stateNode con template:",
            e.message,
          );
        }

        try {
          effectNode = template.ast(effectCode, {
            plugins: ["jsx", "typescript"],
          });
        } catch (e) {
          console.error(
            "❌ Error generando effectNode con template:",
            e.message,
          );
        }

        if (stateCode && effectCode) {
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
                path.node.body.body.unshift(effectNodeNew);
                path.node.body.body.unshift(stateNodeNew);
                modified = true;
              }
            },
          });
        }

        modified = true;
      }

      if (method && model && insertedComponentName === "form") {
        checkMissingImports();

        const lowerMethod = method.toLowerCase();
        const capitalizeMethod = toCapitalize(method);
        const lowerModel = toLower(model);

        let methodService = null;

        if (method === "PUT") methodService = "update";
        else if (method === "POST") methodService = "create";

        const formStateCode = `const [${lowerMethod}${model}, set${capitalizeMethod}${model}] = useState<${model}>();`;
        const handleChangeCode = `const handleChange = (e: any) => {
                 const { name, value } = e.target;
                 set${capitalizeMethod}${model}((prevData) => ({
                   ...prevData,
                   [name]: value,
                 }));
               };`;
        const handleSubmitCode = `const handleSubmit = async (e: React.FormEvent) => {
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

        let stateNode = null;
        let changeNode = null;
        let submitNode = null;

        try {
          stateNode = template.ast(formStateCode, {
            plugins: ["jsx", "typescript"],
          });
        } catch (e) {
          console.error(
            "❌ Error generando stateNode con template:",
            e.message,
          );
        }

        try {
          changeNode = template.ast(handleChangeCode, {
            plugins: ["jsx", "typescript"],
          });
        } catch (e) {
          console.error(
            "❌ Error generando changeNode con template:",
            e.message,
          );
        }

        try {
          submitNode = template.ast(handleSubmitCode, {
            plugins: ["jsx", "typescript"],
          });
        } catch (e) {
          console.error(
            "❌ Error generando submitNode con template:",
            e.message,
          );
        }

        if (stateNode && changeNode && submitNode)
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
