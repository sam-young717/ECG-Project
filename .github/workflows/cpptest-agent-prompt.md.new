You are running in non-interactive (headless) mode inside a GitHub Actions job
on a self-hosted runner. The current working directory is the checked-out
repository. The Parasoft C/C++test analysis has already run and produced a
report at `reports/report.xml`. An MCP server named `cpptest-std-mcp` is
registered and exposes tools for reading violations and rule documentation.

Your task: fix every static analysis violation reported in `reports/report.xml`
and commit the fixes.

Hard rules — read carefully:

1. Do ALL work in the foreground in this single session. Do NOT spawn background
   subagents. Do NOT use `agent` / `general-purpose` / `delegate` style tools
   that return an `agent_id`. If you delegate work to a background agent, this
   process will exit before the agent runs and nothing will be fixed.
2. Use the `cpptest-std-mcp` MCP server for violation data and rule docs
   whenever possible (`get_violations_from_report_file`,
   `get_rule_documentation`, `get_relevant_rules`, etc.).
3. Group violations by rule, fix them with normal file edits, and after each
   logical group commit the changes with `git commit -m "fix(misra): <rule>: <summary>"`.
   Do NOT push — the surrounding workflow handles pushing.
4. Do not modify build files (`CMakeLists.txt`, `Makefile`, `BUILD.bazel`,
   `MODULE.bazel`, `WORKSPACE.bazel`, `project.bdf`, `cpptest-analyze.sh`,
   `cpptestcc-bazel.psrc`) unless the violation is literally inside one of them.
5. Do not modify the `reports/` directory.
6. When you have processed all rules, stop. Print a final one-line summary
   showing total violations fixed and how many commits you made.

Begin now.
