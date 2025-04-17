const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;
const t = require("@babel/types");

const [, , basePath, operation, transactionName] = process.argv;

(async () => {
  try {
    let modified = false;

    if (operation === "modify") {
     //modify
    }

     if (operation === "delete" && transactionName) {
       const capitalized = transactionName.charAt(0).toUpperCase() + transactionName.slice(1);
       const lower = transactionName.charAt(0).toLowerCase() + transactionName.slice(1);
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

         traverse(ast, {
           VariableDeclaration(path) {
             const declarationCode = generate(path.node).code;

             // Eliminar importación de modelo
             if (filePath.includes("models/index.js")) {
               if (declarationCode.includes(`require("./${capitalized}")`)) {
                 path.remove();
                 modified = true;
               }
             }

             // Eliminar importación de rutas
             if (filePath.includes("routes/index.js")) {
               if (declarationCode.includes(`require('./${lower}Routes')`)) {
                 path.remove();
                 modified = true;
               }
             }
           },

           ExpressionStatement(path) {
             const code = generate(path.node).code;

             // Eliminar router.use(...) que contenga el nombre de las rutas
             if (filePath.includes("routes/index.js")) {
               if (code.includes(`router.use(${routeImportName})`)) {
                 path.remove();
                 modified = true;
               }
             }
           },

           ObjectExpression(path) {
             // Eliminar propiedad exportada (como Task,) del export de models
             if (filePath.includes("models/index.js")) {
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
             }
           },
         });

         if (modified) {
           const output = generate(ast, { retainLines: true }, sourceCode);
           fs.writeFileSync(filePath, output.code);
           execSync(`biome format ${filePath} --write`, { stdio: "inherit" });
           console.log(`✅ ${filePath} modificado y formateado.`);
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
