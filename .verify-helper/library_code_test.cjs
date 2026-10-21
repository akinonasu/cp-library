const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const script = fs.readFileSync(path.join(__dirname, 'docs/static/assets/js/library-code.js'), 'utf8');

function page({profile = 'full', limit = 262144, original = 'original\n', bundled = 'bundled\n', href = 'https://example.com/a.hpp.html?q=abc#code', clipboard = true, fallback = true} = {}) {
  const elements = {}, copied = [], selected = [];
  const element = () => ({hidden: false, dataset: {}, attributes: {}, listeners: {}, style: {}, textContent: '',
    addEventListener(name, callback) { this.listeners[name] = callback; },
    setAttribute(name, value) { this.attributes[name] = value; },
    after(input) { elements[input.id] = input; },
    remove() { delete elements[this.id]; },
    focus() {}, select() { selected.push(this.value); }});
  for (const id of ['code-original', 'code-bundled', 'code-bundle', 'code-copy', 'code-copy-source', 'code-size', 'code-status', 'library-code']) elements[id] = element();
  elements['code-copy-source'].value = bundled;
  elements['code-original'].querySelector = () => ({textContent: original});
  elements['code-bundled'].querySelector = () => ({textContent: profile === 'lightweight' ? '\n' + bundled + '\n' : bundled});
  elements['code-bundled'].hidden = true;
  elements['library-code'].dataset = {sizeProfile: profile, codeLimit: String(limit)};
  const toolbar = element();
  toolbar.hidden = true;
  const document = {getElementById: id => elements[id], querySelector: () => toolbar,
    createElement: element, execCommand: () => fallback};
  const navigator = {clipboard: {writeText: async text => {
    if (!clipboard) throw new Error('clipboard unavailable');
    copied.push(text);
  }}};
  vm.runInNewContext(script, {document, navigator, window: {location: {href}}, URL, TextEncoder});
  return {elements, toolbar, copied, selected, click: id => elements[id].listeners.click()};
}

test('full pages keep original and bundle copying', async () => {
  const p = page();
  assert.equal(p.toolbar.hidden, false);
  await p.click('code-copy');
  assert.equal(p.copied[0], '//https://example.com/a.hpp.html\noriginal\n');
  p.click('code-bundle');
  await p.click('code-copy');
  assert.equal(p.copied[1], '//https://example.com/a.hpp.html\nbundled\n');
  assert.equal(p.elements['code-bundle'].attributes['aria-pressed'], 'true');
});

test('lightweight pages show and copy expanded code even while inspecting the original', async () => {
  const p = page({profile: 'lightweight', limit: 65536});
  assert.equal(p.elements['code-original'].hidden, true);
  assert.equal(p.elements['code-bundled'].hidden, false);
  p.click('code-bundle');
  assert.equal(p.elements['code-original'].hidden, false);
  await p.click('code-copy');
  assert.equal(p.copied[0], '//https://example.com/a.hpp.html\nbundled\n');
});

test('exactly 64 KiB is allowed including the normalized URL', async () => {
  const prefix = '//https://example.com/a.hpp.html\n';
  const p = page({profile: 'lightweight', limit: 65536, bundled: 'x'.repeat(65536 - prefix.length)});
  await p.click('code-copy');
  assert.equal(Buffer.byteLength(p.copied[0]), 65536);
  assert.equal(p.elements['code-size'].dataset.overLimit, 'false');
});

test('UTF-8 bytes exceeding 64 KiB cannot reach the clipboard or fallback', async () => {
  const p = page({profile: 'lightweight', limit: 65536, original: 'small', bundled: '円'.repeat(22000)});
  await p.click('code-copy');
  assert.equal(p.copied.length, 0);
  assert.equal(p.selected.length, 0);
  assert.equal(p.elements['code-size'].dataset.overLimit, 'true');
  assert.match(p.elements['code-status'].textContent, /64 KiB/);
});

test('fallback copies the same bounded expanded code', async () => {
  const p = page({profile: 'lightweight', limit: 65536, clipboard: false});
  await p.click('code-copy');
  assert.deepEqual(p.selected, ['//https://example.com/a.hpp.html\nbundled\n']);
  assert.equal(p.elements['code-copy-fallback'], undefined);
});

test('failed fallback leaves selected code for manual copying', async () => {
  const p = page({clipboard: false, fallback: false});
  await p.click('code-copy');
  assert.equal(p.elements['code-copy-fallback'].value, p.selected[0]);
  assert.match(p.elements['code-status'].textContent, /選択済み/);
});

test('the full site also blocks copies exceeding its limit', async () => {
  const p = page({original: 'x'.repeat(262145)});
  await p.click('code-copy');
  assert.equal(p.copied.length, 0);
  assert.match(p.elements['code-status'].textContent, /256 KiB/);
});

test('bundle status reports ordinary dependency expansion', () => {
  const p = page();
  p.elements['code-bundled'].dataset = {sourceBytes: '100'};
  p.click('code-bundle');
  assert.match(p.elements['code-status'].textContent, /依存ヘッダーを展開しました（100 bytes）/);
});
