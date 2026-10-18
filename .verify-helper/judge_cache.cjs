const fs = require('node:fs');
const cache = require('./cache-tool/node_modules/@actions/cache');

async function main() {
    const operation = process.env.INPUT_OPERATION || process.argv[2];
    const file = process.env.INPUT_MANIFEST || process.argv[3];
    if (!['restore', 'save'].includes(operation)) throw new Error('Expected restore or save');
    const manifest = JSON.parse(fs.readFileSync(file, 'utf8'));
    let next = 0;
    await Promise.all(Array.from({length: 2}, async () => {
        while (next < manifest.entries.length) {
            const item = manifest.entries[next++];
            try {
                if (operation === 'restore') {
                    item.restored = await cache.restoreCache(item.paths, item.key);
                    console.log(`${item.url}: ${item.restored ? 'restored' : 'not cached'}`);
                } else if (item.ready && !item.restored) {
                    const id = await cache.saveCache(item.paths, item.key);
                    console.log(`${item.url}: ${id >= 0 ? 'saved' : 'cache unavailable'}`);
                }
            } catch (error) {
                console.log(`::warning::Judge data cache ${operation}: ${error.message}`);
            }
        }
    }));
    if (operation === 'restore') fs.writeFileSync(file, JSON.stringify(manifest));
}
main().catch(error => { console.error(error); process.exitCode = 1; });
