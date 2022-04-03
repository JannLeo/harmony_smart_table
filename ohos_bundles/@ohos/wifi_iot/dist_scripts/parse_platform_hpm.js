const fs = require('fs');
const path = require('path');

function readJSONFile(filePath) {
    return JSON.parse(fs.readFileSync(filePath).toString())
}

function addFeatures(componentFeatures, targetIndex) {
    if (componentFeatures instanceof Array && targetIndex === 0&&componentFeatures.length>0) {
        
        return componentFeatures.map(obj=>{
            const key=Object.keys(obj)[0];
            const value=obj[key];
            return `${key}=${value}`
        })
    }
    return []
}

function parseComponent(components) {
    return components.reduce((result, component) => {
        return result.concat(component.targets.map((target, index) => ({
            name: component.component,
            dir: target,
            optional: component.optional,
            features: addFeatures(component.features, index)
        })))
    }, [])
}

function main(platformPath) {

    const platformJSON = readJSONFile(platformPath);
    const resultJSON = [];
    platformJSON.subsystems.forEach(subsystem => {
        resultJSON.push({
            name: subsystem.subsystem,
            optional: subsystem.optional,
            component: parseComponent(subsystem.components)
        })
    });

    fs.writeFileSync(path.join(process.env.DEP_BUNDLE_BASE, 'subsystems_product.json'), JSON.stringify(resultJSON.sort((a, b) => a.name.localeCompare(b.name)), null, 4));
}

const args = process.argv.splice(2)
const platformPath = process.env.DEP_BUNDLE_BASE ? path.join(process.env.DEP_BUNDLE_BASE, 'build', 'lite', 'platform', args[0], 'platform.json') : path.resolve(args[0]);// hpm dist 调用 或手动指定位置
console.log(platformPath);
main(platformPath);
