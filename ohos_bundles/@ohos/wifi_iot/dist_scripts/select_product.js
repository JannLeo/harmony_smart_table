const fs = require('fs');
const path = require('path');
const execSync = require('child_process').execSync;

function readJSONFile(filePath) {
    return JSON.parse(fs.readFileSync(filePath).toString())
}
function parseList() {
    try {
        const depsoutput = execSync('cd ${DEP_BUNDLE_BASE}&&hpm list').toString();
        const depStringList = depsoutput.substring(depsoutput.indexOf("+--")).split('\n');
        console.log(depStringList)
        const depnamelist = depStringList.map(depString => {
            return depString ? depString.split("@")[1].split('/')[1] : ""
        }).filter(d => d)
        return depnamelist
    } catch (err) {
        console.log(err.message)
        process.exit(-1)
    }
}

function main(subsystemsProductPath) {

    const subsystemsProductJSON = readJSONFile(subsystemsProductPath);
    const selectProduct = []
    const depnameList = parseList()
    console.log(depnameList);
    depnameList.forEach(depname => {
        subsystemsProductJSON.forEach(subsystem => {
            subsystem.component.forEach(component => {
                if (component.name === depname) {
                    selectProduct.push({ ...component, subsystem: subsystem.name })
                }
            })
        });
    })
    const selectProductObj = selectProduct.reduce((selectObj, product) => {
        if (selectObj[product.subsystem]) {
            selectObj[product.subsystem].push({ name: product.name, dir: product.dir, features: product.features })
        } else {
            selectObj[product.subsystem] = [{ name: product.name, dir: product.dir, features: product.features }]
        }
        return selectObj;
    }, {})

    const result = Object.keys(selectProductObj).map(subsystem => {
        return {
            name: subsystem,
            component: selectProductObj[subsystem]
        }
    })
    fs.writeFileSync(path.join(process.env.DEP_BUNDLE_BASE, 'select_product.json'), JSON.stringify(result.sort((a, b) => a.name.localeCompare(b.name)), null, 4));
}

const subsystemsProductPath = path.join(process.env.DEP_BUNDLE_BASE, 'subsystems_product.json')
console.log(subsystemsProductPath);
main(subsystemsProductPath);
