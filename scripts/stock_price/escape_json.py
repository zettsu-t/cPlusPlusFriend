import json
escaped = json.load(open("escaped.json", "r"))

with open("asis_escaped.json", "w") as f:
  json.dump(escaped, f)

with open("ill_escaped.json", "w") as f:
  f.write(json.dumps(escaped).encode("ascii").decode("unicode-escape"))

with open("well_escaped.json", "w") as f:
  f.write(json.dumps(escaped).encode("ascii").decode("unicode-escape").replace("\\", "\\\\"))

print(json.load(open("well_escaped.json", "r")))

try:
    print(json.load(open("ill_escaped.json", "r")))
except json.decoder.JSONDecodeError:
    print("Cannot read ill_escaped.json")
