import { afterAll, describe, it } from "vitest";
import { execSync } from "node:child_process";
import path from "node:path";

function runQuiet(cmd, opts = {}) {
  try {
    execSync(cmd, {
      ...opts,
      stdio: ["ignore", "pipe", "pipe"], // capture stdout + stderr
    });
  } catch (err) {
    if (err.stdout) process.stdout.write(err.stdout);
    if (err.stderr) process.stderr.write(err.stderr);
    throw err;
  }
}

const ROOT = path.resolve(__dirname, "../src/lib");

describe("C unit tests", () => {
  afterAll(() => {
    execSync(
      "rm ./*.out",
      {
        cwd: ROOT,
        stdio: "inherit"
      }
    );
    console.log("Deleted test binaries");
  });

  it("cfns.c", () => {
    // compile
    runQuiet(
      "gcc cfns.test.c cfns.c -lcriterion -o cfns.test.out",
      {
        cwd: ROOT,
      }
    );

    // run
    runQuiet(
      "./cfns.test.out --verbose",
      {
        cwd: ROOT,
      }
    );

    runQuiet(
      "valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./cfns.test.out --verbose",
      {
        cwd: ROOT,
      }
    );
  });
});

