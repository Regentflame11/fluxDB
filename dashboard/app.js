// ─────────────────────────────────────────────────────
//  FluxDB Dashboard — app.js
// ─────────────────────────────────────────────────────

const API_BASE = `http://${window.location.hostname}:8080/api`;
const REFRESH_MS = 3000;

let hitrateChart, keysChart;
let hitHistory  = [];
let missHistory = [];
let keysHistory = [];
let timeLabels  = [];
const MAX_PTS   = 20;
let allKeys     = [];
let activeKey   = null;

const $ = id => document.getElementById(id);

document.addEventListener('DOMContentLoaded', () => {
    initCharts();
    fetchStats();
    fetchKeys();
    setInterval(fetchStats, REFRESH_MS);
    setInterval(fetchKeys, REFRESH_MS * 2);
    setupConsole();
    setupKeySearch();
    setupModal();
});

// ─────────────────────────────────────────────────────
//  Charts — line style legend (no boxes)
// ─────────────────────────────────────────────────────
function initCharts() {
    const baseOpts = {
        responsive: true,
        maintainAspectRatio: false,
        interaction: { mode: 'index', intersect: false },
        plugins: {
            legend: {
                display: true,
                labels: {
                    color: '#6c7a99',
                    font: { size: 11, family: 'JetBrains Mono' },
                    usePointStyle: true,   // show line/point instead of box
                    pointStyle: 'line',    // line in legend
                    boxWidth: 20,
                    boxHeight: 0,
                    padding: 16
                }
            },
            tooltip: {
                backgroundColor: '#181c26',
                borderColor: '#2a3045',
                borderWidth: 1,
                titleColor: '#cdd6f4',
                bodyColor: '#6c7a99',
                titleFont: { family: 'JetBrains Mono', size: 11 },
                bodyFont:  { family: 'JetBrains Mono', size: 11 },
                padding: 10
            }
        },
        scales: {
            x: {
                ticks: {
                    color: '#3d4663',
                    font: { size: 10, family: 'JetBrains Mono' },
                    maxTicksLimit: 6
                },
                grid: { color: '#1f2535' }
            },
            y: {
                ticks: {
                    color: '#3d4663',
                    font: { size: 10, family: 'JetBrains Mono' }
                },
                grid: { color: '#1f2535' },
                beginAtZero: true
            }
        },
        elements: {
            line: { tension: 0.3 },
            point: { radius: 2, hoverRadius: 4 }
        },
        animation: { duration: 300 }
    };

    hitrateChart = new Chart($('chart-hitrate'), {
        type: 'line',
        data: {
            labels: timeLabels,
            datasets: [
                {
                    label: 'Hits',
                    data: hitHistory,
                    borderColor: '#a6e3a1',
                    backgroundColor: 'rgba(166,227,161,0.08)',
                    fill: true,
                    borderWidth: 1.5,
                    pointBackgroundColor: '#a6e3a1',
                    pointStyle: 'circle'
                },
                {
                    label: 'Misses',
                    data: missHistory,
                    borderColor: '#f38ba8',
                    backgroundColor: 'rgba(243,139,168,0.06)',
                    fill: true,
                    borderWidth: 1.5,
                    pointBackgroundColor: '#f38ba8',
                    pointStyle: 'circle'
                }
            ]
        },
        options: baseOpts
    });

    keysChart = new Chart($('chart-keys'), {
        type: 'line',
        data: {
            labels: timeLabels,
            datasets: [{
                label: 'Keys',
                data: keysHistory,
                borderColor: '#4a8ef5',
                backgroundColor: 'rgba(74,142,245,0.07)',
                fill: true,
                borderWidth: 1.5,
                pointBackgroundColor: '#4a8ef5',
                pointStyle: 'circle'
            }]
        },
        options: baseOpts
    });
}

// ─────────────────────────────────────────────────────
//  Stats Polling
// ─────────────────────────────────────────────────────
async function fetchStats() {
    try {
        const res  = await fetch(`${API_BASE}/stats`);
        const data = await res.json();
        updateStats(data);
        setStatus(true);
    } catch {
        setStatus(false);
    }
}

function updateStats(data) {
    $('stat-keys').textContent      = data.key_count;
    $('stat-hitrate').textContent   = data.hit_rate.toFixed(1) + '%';
    $('stat-hits').textContent      = fmt(data.hits);
    $('stat-misses').textContent    = fmt(data.misses);
    $('stat-evictions').textContent = fmt(data.evictions);
    $('stat-maxkeys').textContent   = `max ${data.max_size}`;

    const rate = data.hit_rate;
    const meta = $('hitrate-meta');
    if      (rate >= 80) { meta.textContent = 'excellent'; meta.style.color = '#a6e3a1'; }
    else if (rate >= 50) { meta.textContent = 'fair';      meta.style.color = '#f9e2af'; }
    else                 { meta.textContent = 'low';       meta.style.color = '#f38ba8'; }

    const now = new Date().toLocaleTimeString([], { hour12: false });
    timeLabels.push(now);
    hitHistory.push(data.hits);
    missHistory.push(data.misses);
    keysHistory.push(data.key_count);

    if (timeLabels.length > MAX_PTS) {
        timeLabels.shift();
        hitHistory.shift();
        missHistory.shift();
        keysHistory.shift();
    }

    hitrateChart.update();
    keysChart.update();

    $('last-update').textContent = now;
}

// ─────────────────────────────────────────────────────
//  Key Browser
// ─────────────────────────────────────────────────────
async function fetchKeys() {
    try {
        const res = await fetch(`${API_BASE}/keys`);
        allKeys   = await res.json();
        renderKeys(allKeys);
    } catch { /* silent */ }
}

function renderKeys(keys) {
    const list    = $('key-list');
    const q       = $('key-search').value.toLowerCase();
    const filtered = keys.filter(k => k.toLowerCase().includes(q));

    if (filtered.length === 0) {
        list.innerHTML = '<div class="key-empty">no keys found</div>';
        return;
    }

    list.innerHTML = filtered.map(k => `
        <div class="key-item" data-key="${esc(k)}">
            <span>${esc(k)}</span>
            <span class="key-arrow">-&gt;</span>
        </div>
    `).join('');

    list.querySelectorAll('.key-item').forEach(el => {
        el.addEventListener('click', () => openModal(el.dataset.key));
    });
}

function setupKeySearch() {
    $('key-search').addEventListener('input', () => renderKeys(allKeys));
    $('btn-refresh-keys').addEventListener('click', fetchKeys);
}

// ─────────────────────────────────────────────────────
//  Console
// ─────────────────────────────────────────────────────
function setupConsole() {
    const input = $('console-input');

    const run = () => {
        const cmd = input.value.trim();
        if (!cmd) return;
        input.value = '';
        runCmd(cmd);
    };

    input.addEventListener('keydown', e => { if (e.key === 'Enter') run(); });
    $('btn-run').addEventListener('click', run);

    $('btn-clear-console').addEventListener('click', () => {
        $('console-output').innerHTML = '<div class="console-line info">console cleared</div>';
    });

    document.querySelectorAll('.hint').forEach(el => {
        el.addEventListener('click', () => {
            input.value = el.dataset.cmd;
            input.focus();
        });
    });
}

async function runCmd(cmd) {
    log('cmd', `> ${cmd}`);
    try {
        const res  = await fetch(`${API_BASE}/query`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ cmd })
        });
        const data = await res.json();

        if (data.error) {
            log('error', `ERR: ${data.error}`);
        } else if (data.result === null) {
            log('null', '(nil)');
        } else if (Array.isArray(data.result)) {
            if (data.result.length === 0) log('null', '(empty)');
            else data.result.forEach((item, i) => log('result', `${i + 1}  ${item}`));
        } else if (typeof data.result === 'object') {
            Object.entries(data.result).forEach(([k, v]) => log('result', `${k}: ${v}`));
        } else {
            log('result', String(data.result));
        }

        fetchStats();
        fetchKeys();
    } catch {
        log('error', 'ERR: could not reach FluxDB server');
    }
}

function log(type, text) {
    const out = $('console-output');
    const div = document.createElement('div');
    div.className = `console-line ${type}`;
    div.textContent = text;
    out.appendChild(div);
    out.scrollTop = out.scrollHeight;
}

// ─────────────────────────────────────────────────────
//  Modal
// ─────────────────────────────────────────────────────
function setupModal() {
    $('modal-close').addEventListener('click',  closeModal);
    $('modal-cancel').addEventListener('click', closeModal);
    $('modal-overlay').addEventListener('click', e => {
        if (e.target === $('modal-overlay')) closeModal();
    });
    $('modal-del').addEventListener('click', async () => {
        if (!activeKey) return;
        await runCmd(`DEL ${activeKey}`);
        closeModal();
        fetchKeys();
    });
}

async function openModal(key) {
    activeKey = key;
    $('modal-key-name').textContent = key;
    $('modal-value').textContent    = '...';
    $('modal-ttl').textContent      = '...';
    $('modal-overlay').classList.add('active');

    const [vr, tr] = await Promise.all([
        fetch(`${API_BASE}/query`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ cmd: `GET ${key}` }) }),
        fetch(`${API_BASE}/query`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ cmd: `TTL ${key}` }) })
    ]);

    const vd = await vr.json();
    const td = await tr.json();

    $('modal-value').textContent = vd.result !== null ? vd.result : '(nil)';

    const t = td.result;
    if      (t === -1) $('modal-ttl').textContent = 'no expiry';
    else if (t === -2) $('modal-ttl').textContent = 'key not found';
    else               $('modal-ttl').textContent = `${t}s remaining`;
}

function closeModal() {
    $('modal-overlay').classList.remove('active');
    activeKey = null;
}

// ─────────────────────────────────────────────────────
//  Status
// ─────────────────────────────────────────────────────
function setStatus(online) {
    const dot  = $('status-indicator');
    const text = $('status-text');
    dot.className  = `status-indicator ${online ? 'online' : 'offline'}`;
    text.textContent = online ? 'connected' : 'disconnected';
    text.style.color = online ? '#a6e3a1' : '#f38ba8';
}

// ─────────────────────────────────────────────────────
//  Utils
// ─────────────────────────────────────────────────────
function fmt(n) {
    if (n >= 1_000_000) return (n / 1_000_000).toFixed(1) + 'M';
    if (n >= 1_000)     return (n / 1_000).toFixed(1) + 'K';
    return String(n);
}

function esc(s) {
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}



