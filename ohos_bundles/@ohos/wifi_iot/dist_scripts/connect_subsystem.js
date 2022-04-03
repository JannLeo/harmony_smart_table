const path = require("path");
const fs = require("fs");

function readJSONFile(filePath) {
    return JSON.parse(fs.readFileSync(filePath).toString())
}
function main(src) {
    const args = process.argv.splice(2)
    const filename = args[0];
    const mainProduct = readJSONFile(path.join(src, 'product.template.json'));
    const subsystem = readJSONFile(path.join(src, 'select_product.json'))
    const productPath = path.join(src, 'build', 'lite', 'product', `${filename}.json`)
    mainProduct.subsystem = subsystem;
    fs.writeFileSync(productPath, JSON.stringify(mainProduct, null, 4))
    console.log(`comple json generated at ${productPath}, start compile`)
}


const src = path.join(path.resolve(process.env.DEP_BUNDLE_BASE));
console.log(src);
main(src);
