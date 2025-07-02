const { execSync } = require("child_process");
const fs = require("fs");

// Recibir argumentos dinámicamente
const args = process.argv.slice(2);
const [editorScript, ...editorArgs] = args;

try {
    if (!fs.existsSync("node_modules")) {
        console.log("🔄 node_modules no encontrado. Ejecutando bun install...");
        execSync("bun install", { stdio: "inherit" });
    }

    console.log("🚀 Ejecutando editor...");
    execSync(`bun run ${editorScript} ${editorArgs.join(" ")}`, {
        stdio: "inherit",
    });
} catch (error) {
    console.error("❌ Error ejecutando el editor:", error);
    process.exit(1);
}
