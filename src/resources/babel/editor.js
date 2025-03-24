// editor.js
const fs = require("fs");
const parser = require("@babel/parser");
const generate = require("@babel/generator").default;
const traverse = require("@babel/traverse").default;

const [, , filePath, operation, referenceId, position, payload] = process.argv;

const sourceCode = fs.readFileSync(filePath, "utf-8");
const ast = parser.parse(sourceCode, {
  sourceType: "module",
  plugins: ["jsx", "typescript"],
});

const fragmentAst = payload
  ? parser.parseExpression(JSON.parse(payload), { plugins: ["jsx"] })
  : null;

let modified = false;

if (operation === "modify" || operation === "delete") {
  traverse(ast, {
    JSXElement(path) {
      const attr = path.node.openingElement.attributes.find(
        (a) => a.type === "JSXAttribute" && a.name.name === "data-id"
      );
      if (!attr || attr.value.value !== referenceId) return;

      if (operation === "modify" && fragmentAst) {
        path.replaceWith(fragmentAst);
      } else if (operation === "delete") {
        path.remove();
      }

      modified = true;
      path.stop();
    }
  });
}

if (operation === "insert" && fragmentAst) {
  if (referenceId === "none" && position === "inner") {
    // Caso base: insertar dentro del primer <div>
    traverse(ast, {
      JSXElement(path) {
        if (
          path.node.openingElement.name.name === "div" &&
          !modified
        ) {
          path.node.children.push(fragmentAst);
          modified = true;
          path.stop();
        }
      }
    });
  } else {
    // Inserción basada en referenceId y posición
    traverse(ast, {
      JSXElement(path) {
        const attr = path.node.openingElement.attributes.find(
          (a) => a.type === "JSXAttribute" && a.name.name === "data-id"
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
fs.writeFileSync(filePath, output.code);
