(() => {
  const original = document.getElementById('code-original');
  const bundled = document.getElementById('code-bundled');
  const bundle = document.getElementById('code-bundle');
  const status = document.getElementById('code-status');
  const container = document.getElementById('library-code');
  const lightweight = container.dataset.sizeProfile === 'lightweight';
  const limit = Number(container.dataset.codeLimit);
  const codeSize = document.getElementById('code-size');
  const source = () => {
    const code = lightweight ? document.getElementById('code-copy-source').value : (original.hidden ? bundled : original).querySelector('code').textContent;
    const url = new URL(window.location.href);
    url.search = '';
    url.hash = '';
    return `//${url.href}\n${code}`;
  };
  const measure = () => {
    const bytes = new TextEncoder().encode(source()).length;
    codeSize.textContent = `${bytes} B${limit ? ` / ${limit / 1024} KiB` : ''}`;
    codeSize.dataset.overLimit = String(Boolean(limit && bytes > limit));
    return bytes;
  };
  if (lightweight) {
    original.hidden = true;
    bundled.hidden = false;
    bundle.setAttribute('aria-pressed', 'true');
    bundle.textContent = 'Unbundle';
  }
  document.querySelector('.code-toolbar').hidden = false;
  measure();
  bundle.addEventListener('click', () => {
    document.getElementById('code-copy-fallback')?.remove();
    original.hidden = !original.hidden;
    bundled.hidden = !original.hidden;
    bundle.setAttribute('aria-pressed', String(original.hidden));
    bundle.textContent = original.hidden ? 'Unbundle' : 'Bundle';
    const details = bundled.dataset.sourceBytes ? `（${bundled.dataset.sourceBytes} bytes）` : '';
    status.textContent = original.hidden ? `依存ヘッダーを展開しました${details}` : '';
    measure();
  });
  document.getElementById('code-copy').addEventListener('click', async () => {
    document.getElementById('code-copy-fallback')?.remove();
    const text = source();
    const bytes = measure();
    if (limit && bytes > limit) {
      status.textContent = `参照URLを含めて${limit / 1024} KiBを超えるためコピーできません`;
      return;
    }
    let copied = false;
    try {
      await navigator.clipboard.writeText(text);
      copied = true;
    } catch {
      const input = document.createElement('textarea');
      input.id = 'code-copy-fallback';
      input.value = text;
      input.readOnly = true;
      input.setAttribute('aria-label', 'コピー用コード');
      input.style.cssText = 'width:100%;min-height:12rem;box-sizing:border-box';
      document.querySelector('.code-toolbar').after(input);
      input.focus();
      input.select();
      try {
        copied = document.execCommand('copy');
      } catch {}
      if (copied) {
        input.remove();
        document.getElementById('code-copy').focus();
      }
    }
    status.textContent = copied ? 'コピーしました' : '選択済みのコードをコピーしてください';
  });
})();
