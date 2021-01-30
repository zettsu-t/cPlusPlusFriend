import json
import re

escaped = json.load(open("escaped.json", "r"))

with open("asis_escaped.json", "w") as f:
  json.dump(escaped, f, indent=True)

with open("ill_escaped.json", "w") as f:
  f.write(json.dumps(escaped, indent=True).encode("ascii").decode("unicode-escape"))

with open("well_escaped.json", "w") as f:
  f.write(json.dumps(escaped, indent=True).encode("ascii").decode("unicode-escape").replace("\\", "\\\\"))


try:
    print(json.load(open("ill_escaped.json", "r")))
except json.decoder.JSONDecodeError:
    print("Cannot read ill_escaped.json")

print(json.load(open("well_escaped.json", "r")))


def read_escape_backslash(filename):
  with open(filename, "r") as f:
    lines = f.readlines()
    lines = [re.sub(r"(?<!\\)(\\(\\\\)*)(?!\\)", r"\1\1", x) for x in lines]
  return json.loads("\n".join(lines))

print(read_escape_backslash("ill_escaped.json"))
print(read_escape_backslash("well_escaped.json"))
