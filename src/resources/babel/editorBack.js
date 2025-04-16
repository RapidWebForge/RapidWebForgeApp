const fs = require("fs");
const { execSync } = require("child_process");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;
const t = require("@babel/types");
const template = require("@babel/template").default;

const [, , filePath, operation, referenceId, ...rest] = process.argv;

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

          if (operation === "modify" && fragmentAst) {
            // MODIFY
            path.replaceWith(t.cloneNode(fragmentAst, true));
          } else if (operation === "delete") {
            // DELETE
          }

          modified = true;
          path.stop();
        },
      });
    }

    if (operation === "insert" && fragmentAst) {
      // INSERT
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
