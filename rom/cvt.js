import fs from "fs";

const f = fs.readFileSync("meshboy.gb").subarray(0, 0x4000);
console.log(`#pragma once
uint8_t rom[0x4000] = { ${f.map(x => x.toString()).join(", ")} };`)