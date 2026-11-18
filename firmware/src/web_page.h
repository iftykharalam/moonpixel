// The control page served by the lamp. This is the SOURCE: tools/gen_web.py compresses it into
// web_page_gz.h on every build, and that compressed copy is what the lamp actually sends.
//
// Built for a weak Wi-Fi link: changes are applied on screen at once, merged into one request,
// sent one at a time (the newest value wins) and retried automatically if a request is lost.
#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0d0f14"><title>MoonPixel</title>
<link rel="icon" type="image/svg+xml" href="/icon.svg"><link rel="apple-touch-icon" href="/apple-touch-icon.png">
<link rel="manifest" href="/manifest.json">
<meta name="apple-mobile-web-app-capable" content="yes"><meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent"><meta name="apple-mobile-web-app-title" content="MoonPixel">
<style>
:root{--bg:#0d0f14;--card:#171a21;--tile:#20242c;--line:#2a2f39;--txt:#eceef2;--mut:#8b93a1;--acc:#ffd27a}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;background:var(--bg);color:var(--txt);font:15px/1.4 -apple-system,system-ui,sans-serif}
main{max-width:440px;margin:0 auto;padding:max(16px,env(safe-area-inset-top)) max(16px,env(safe-area-inset-right)) max(48px,env(safe-area-inset-bottom)) max(16px,env(safe-area-inset-left))}
html,body{overscroll-behavior:none;-webkit-user-select:none;user-select:none}
input{-webkit-user-select:text;user-select:text}
header{display:flex;align-items:center;gap:12px;margin:6px 0 18px}
h1{font-size:21px;margin:0;font-weight:650}
.grow{flex:1}
#net{font-size:12px;color:var(--mut);display:flex;align-items:center;gap:6px;margin-top:2px}
#net i{width:8px;height:8px;border-radius:50%;background:#666}
#net.ok i{background:#4ade80}#net.busy i{background:#fbbf24}#net.bad i{background:#f87171}
#pwr{width:56px;height:56px;border-radius:50%;border:0;background:var(--tile);color:var(--mut);transition:.2s;display:grid;place-items:center}
#pwr.on{background:var(--acc);color:#2a1d00;box-shadow:0 0 28px #ffd27a70}
.card{background:var(--card);border-radius:20px;padding:18px;margin-bottom:14px}
.wheel{position:relative;width:min(76vw,290px);aspect-ratio:1;margin:4px auto 0;border-radius:50%;touch-action:none;
 background:radial-gradient(circle closest-side,#fff,#fff0),conic-gradient(red,#ff0,lime,cyan,blue,#f0f,red);transition:opacity .3s}
.wheel.dim{opacity:.35}
.knob{position:absolute;width:34px;height:34px;margin:-17px 0 0 -17px;border-radius:50%;border:3px solid #fff;
 box-shadow:0 2px 8px #000a;pointer-events:none}
.hint{text-align:center;color:var(--mut);font-size:12.5px;margin:10px 0 0}
.sw{display:grid;grid-template-columns:repeat(5,34px);gap:12px 16px;justify-content:center;margin-top:14px}
.sw b{width:34px;height:34px;border-radius:50%;border:2px solid #ffffff2a;cursor:pointer}
.lab{display:flex;justify-content:space-between;color:var(--mut);font-size:13px;margin:2px 0 4px}
.lab b{color:var(--txt);font-weight:600}
input[type=range]{width:100%;-webkit-appearance:none;appearance:none;height:34px;background:none;margin:0 0 8px}
input[type=range]::-webkit-slider-runnable-track{height:10px;border-radius:5px;background:linear-gradient(90deg,#2b2f38,var(--acc))}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:28px;height:28px;margin-top:-9px;border-radius:50%;background:#fff;box-shadow:0 1px 5px #000a}
input[type=range]::-moz-range-track{height:10px;border-radius:5px;background:linear-gradient(90deg,#2b2f38,var(--acc))}
input[type=range]::-moz-range-thumb{width:26px;height:26px;border:0;border-radius:50%;background:#fff}
.fx{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
.fx button{background:var(--tile);border:2px solid transparent;border-radius:16px;color:var(--txt);padding:13px 4px 11px;font-size:13px}
.fx button i{display:block;width:30px;height:30px;border-radius:50%;margin:0 auto 7px}
.i0{background:radial-gradient(circle at 35% 35%,#fffaf0,#ffd99a 70%)}
.i4{background:radial-gradient(circle,#ffe4a8 0 34%,#ffb84d66 56%,#0000 72%)}
.i1{background:var(--cur,#ffc880)}
.i3{background:radial-gradient(circle,var(--cur,#ffc880) 0 38%,#0000 72%)}
.i2{background:conic-gradient(#f44,#fd4,#4d6,#4df,#46f,#e4f,#f44)}
.i5{background:linear-gradient(#ffc24d,#ff7a1a 55%,#c42a12)}
.i6{background:radial-gradient(circle at 50% 62%,#fff3b8,#ff9a2e 45%,#0000 72%)}
.i7{background:linear-gradient(150deg,#2fd6d9,#0a4fd1)}
.i8{background:linear-gradient(150deg,#3dff9a,#1fb8a8 50%,#7b3fe4)}
.i9{background:radial-gradient(circle,#ff7a1a,#b3160a 62%,#3a0500)}
.i10{background:linear-gradient(#1b2080,#5a2a8c 65%,#9a4a8a)}
.fx .wide{grid-column:1/-1;display:flex;align-items:center;gap:12px;padding:12px 14px;font-size:14px;text-align:left}
.fx .wide i{width:64px;height:30px;border-radius:10px;margin:0;flex:none}
.i12{background:radial-gradient(at 15% 20%,#ff5fa2,#0000 65%),radial-gradient(at 85% 20%,#5f8bff,#0000 65%),radial-gradient(at 15% 85%,#ffc24d,#0000 65%),radial-gradient(at 85% 85%,#3ddc97,#0000 65%),#333}
.i11{background:conic-gradient(#ffb3b3,#ffe0a3,#b8f5c8,#b3d4ff,#dcbcff,#ffb3b3)}
.moon{display:inline-block;width:22px;height:22px;border-radius:50%;vertical-align:-4px;margin-right:9px;
 background:radial-gradient(circle at 33% 33%,#fff7de,#f1d28a 55%,#c49648);box-shadow:0 0 12px #ffd27a66}
.fx button.sel{border-color:var(--acc);background:#2b2616}
summary{list-style:none;cursor:pointer;display:flex;justify-content:space-between;color:var(--txt);font-weight:600}
summary::-webkit-details-marker{display:none}
summary:after{content:"›";color:var(--mut);transition:.2s}details[open] summary:after{transform:rotate(90deg)}
.set label{display:block;color:var(--mut);font-size:13px;margin:16px 0 0}
select,input[type=text],input[type=password],.btn{width:100%;font-size:16px;padding:12px;border-radius:12px;border:1px solid var(--line);background:var(--tile);color:var(--txt);margin-top:6px}
.btn{border:0;text-align:center;text-decoration:none;display:block}
.pri{background:var(--acc);color:#2a1d00;font-weight:650}.warn{background:#4a1f22;color:#ffb4b4}
.row{display:flex;gap:8px}.row .btn{width:96px;flex:none}
.mut{color:var(--mut);font-size:12.5px;margin:8px 0 0}
.nrow{display:flex;justify-content:space-between;align-items:center;padding:6px 6px 6px 14px;background:var(--tile);border-radius:12px;margin-top:6px}
.nrow button{border:0;background:none;color:#ffb4b4;font-size:14px;padding:8px 10px}
#wst{margin:12px 0 0;line-height:1.6;font-size:14px}
</style></head><body><main>

<header>
 <div class="grow"><h1><i class="moon"></i>MoonPixel</h1><div id="net"><i></i><span>Connecting…</span></div></div>
 <button id="pwr" aria-label="Power" onclick="send({on:S.on?0:1})"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.4" stroke-linecap="round"><path d="M12 3v8M6.3 6.3a8 8 0 1 0 11.4 0"/></svg></button>
</header>

<div class="card">
 <div class="wheel" id="wheel"><div class="knob" id="knob"></div></div>
 <p class="hint" id="hint">Drag on the wheel to pick a colour</p>
 <div class="sw" id="sw"></div>
</div>

<div class="card"><div class="fx" id="fx"></div></div>

<div class="card">
 <div class="lab">Brightness <b id="briV"></b></div>
 <input type="range" id="bri" min="1" max="255">
 <div id="spdBox"><div class="lab">Speed <b id="spdV"></b></div>
 <input type="range" id="spd" min="1" max="10"></div>
</div>

<div class="card"><details><summary>Wi-Fi &amp; settings</summary><div class="set">
 <div id="wst"></div>
 <label>Saved networks <span style="color:var(--mut)">(the lamp joins the strongest one)</span></label>
 <div id="nets"></div>
 <label>Add a network (up to 3)</label>
 <div class="row"><select id="ssid"><option value="">Tap Scan…</option></select><button class="btn" onclick="scan()">Scan</button></div>
 <input type="text" id="ssidTxt" placeholder="…or type the network name" autocapitalize="off" autocorrect="off">
 <input type="password" id="pass" placeholder="Password">
 <button class="btn pri" onclick="join()">Save network</button>
 <label>Transmit power</label><select id="tx" onchange="send({tx:this.value})"></select>
 <label>Long-range Wi-Fi</label>
 <select id="lr" onchange="restartSet({range:this.value})"><option value="1">On – best for a weak signal</option><option value="0">Off – normal speed</option></select>
 <label>Bluetooth</label>
 <select id="ble" onchange="restartSet({ble:this.value})"><option value="0">Off – best Wi-Fi</option><option value="1">On – control from a BLE app</option></select>
 <p class="mut">Changing long-range Wi-Fi or Bluetooth restarts the lamp (about a minute to reconnect).</p>
 <a class="btn" href="/diag" target="_blank" style="margin-top:16px">Diagnostics</a>
 <button class="btn warn" onclick="if(confirm('Forget ALL saved Wi-Fi networks?'))post('/api/forget',{})">Forget all Wi-Fi</button>
</div></details></div>

</main><script>
const $=i=>document.getElementById(i);
let S={on:1,mode:0,color:'FFC880',bri:80,spd:3},pend=null,flying=false,touched=0,drag=false;
const FX=[[12,'Gradient – colours circling the moon'],[0,'Moon'],[4,'Moon glow'],[1,'Colour'],[3,'Breathe'],[6,'Candle'],[5,'Sunset'],[9,'Ember'],[2,'Rainbow'],[11,'Pastel'],[7,'Ocean'],[8,'Aurora'],[10,'Twilight']];
const SW=['FFC880','FFFFFF','FF3B30','FF9500','FFD60A','34C759','00C7BE','0A84FF','5E5CE6','FF2D92'];
const ANIM=[2,3,4,5,6,7,8,9,10,11,12],COLOURED=[1,3];

function tfetch(u,o={}){const c=new AbortController(),t=setTimeout(()=>c.abort(),9000);return fetch(u,{...o,signal:c.signal}).finally(()=>clearTimeout(t))}
function post(u,d){return tfetch(u,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(d)})}
function net(c,t){$('net').className=c;$('net').lastChild.textContent=t}

// Apply on screen now; merge into one pending request; send one at a time; retry if lost.
function send(d){for(const k in d)S[k]=k=='color'?d[k]:+d[k];pend=Object.assign(pend||{},d);touched=Date.now();draw();flush()}
function flush(){if(flying||!pend)return;const d=pend;pend=null;flying=true;net('busy','Saving…');
 post('/api/set',d).then(r=>r.json()).then(s=>{flying=false;if(!pend&&!drag)take(s);net('ok','Connected');flush()})
  .catch(()=>{flying=false;pend=Object.assign(d,pend||{});net('bad','Weak signal – retrying…');setTimeout(flush,1500)})}
function restartSet(d){if(confirm('The lamp will restart to apply this. Continue?'))send(d);else draw()}
function take(s){S=s;draw()}

function hexHS(h){const n=parseInt(h,16),r=(n>>16&255)/255,g=(n>>8&255)/255,b=(n&255)/255,mx=Math.max(r,g,b),d=mx-Math.min(r,g,b);
 let H=0;if(d){H=mx==r?((g-b)/d)%6:mx==g?(b-r)/d+2:(r-g)/d+4;H*=60;if(H<0)H+=360}return[H,mx?d/mx:0]}
function hsHex(H,s){const f=n=>{const k=(n+H/60)%6;return Math.round(255*(1-s*Math.max(0,Math.min(k,4-k,1))))};
 return[f(5),f(3),f(1)].map(v=>v.toString(16).padStart(2,'0')).join('').toUpperCase()}
function knob(hex){const[H,s]=hexHS(hex),a=H*Math.PI/180,k=$('knob');
 k.style.left=50+Math.sin(a)*s*50+'%';k.style.top=50-Math.cos(a)*s*50+'%';k.style.background='#'+hex}
function pick(e){const r=$('wheel').getBoundingClientRect(),x=e.clientX-r.left-r.width/2,y=e.clientY-r.top-r.height/2;
 const s=Math.min(1,Math.hypot(x,y)/(r.width/2)),H=(Math.atan2(x,-y)*180/Math.PI+360)%360,hex=hsHex(H,s);
 const d={color:hex};if(!COLOURED.includes(S.mode))d.mode=1;if(!S.on)d.on=1;send(d)}
const W=$('wheel');
W.addEventListener('pointerdown',e=>{drag=true;W.setPointerCapture(e.pointerId);pick(e)});
W.addEventListener('pointermove',e=>{if(drag)pick(e)});
W.addEventListener('pointerup',()=>drag=false);W.addEventListener('pointercancel',()=>drag=false);

$('sw').innerHTML=SW.map(c=>`<b style="background:#${c}" onclick="swatch('${c}')"></b>`).join('');
function swatch(c){const d={color:c};if(!COLOURED.includes(S.mode))d.mode=1;if(!S.on)d.on=1;send(d)}
$('fx').innerHTML=FX.map(([m,n])=>`<button id="fx${m}"${m==12?' class="wide"':''} onclick="send({mode:${m},on:1})"><i class="i${m}"></i>${n}</button>`).join('');
for(const id of['bri','spd']){$(id).oninput=e=>{$(id+'V').textContent=e.target.value;send({[id]:e.target.value})}}

function q(r){return r>-60?'great':r>-70?'good':r>-78?'weak':'very weak'}
function esc(t){return String(t).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]))}
function draw(){
 $('pwr').classList.toggle('on',!!S.on);
 if(document.activeElement!=$('bri')){$('bri').value=S.bri}$('briV').textContent=S.bri;
 if(document.activeElement!=$('spd')){$('spd').value=S.spd}$('spdV').textContent=S.spd;
 $('spdBox').style.display=ANIM.includes(S.mode)?'':'none';
 FX.forEach(([m])=>$('fx'+m).classList.toggle('sel',S.mode==m));
 document.body.style.setProperty('--cur','#'+S.color);knob(S.color);W.classList.toggle('dim',!COLOURED.includes(S.mode));
 $('hint').textContent=COLOURED.includes(S.mode)?'Drag on the wheel to pick a colour':'Touch the wheel to switch to a solid colour';
 if(S.txLevels&&!$('tx').options.length)S.txLevels.forEach((v,i)=>$('tx').add(new Option(v=='Auto'?'Auto (recommended)':v+' dBm',i)));
 if('tx'in S)$('tx').value=S.tx;if('lr'in S)$('lr').value=S.lr;if('ble'in S)$('ble').value=S.ble;
 if(S.nets)$('nets').innerHTML=S.nets.map(n=>'<div class="nrow"><span>'+esc(n)+'</span><button data-n="'+esc(n)+'" onclick="forgetNet(this.dataset.n)">Remove</button></div>').join('')||'<p class="mut">None saved yet.</p>';
 if('ssid'in S)$('wst').innerHTML=S.connected?'Connected to <b>'+esc(S.ssid)+'</b> · '+S.rssi+' dBm ('+q(S.rssi)+')<br>Address: '+S.ip
  :(S.ssid?'Connecting to <b>'+esc(S.ssid)+'</b>… '+esc(S.lastError||''):'No network saved yet.')}
function forgetNet(n){if(confirm('Forget '+n+'?'))post('/api/forget',{ssid:n}).then(()=>setTimeout(poll,3000)).catch(()=>alert('No reply – try again'))}
function poll(){if(flying||pend||drag||Date.now()-touched<4000)return;
 tfetch('/api/state').then(r=>r.json()).then(s=>{take(s);net('ok','Connected')}).catch(()=>net('bad','No reply – weak signal'))}
function scan(){$('ssid').innerHTML='<option>Scanning…</option>';tfetch('/api/scan').then(r=>r.json()).then(l=>{
 $('ssid').innerHTML=l.length?'':'<option value="">Nothing found</option>';
 l.forEach(n=>$('ssid').add(new Option(n.ssid+' · '+n.rssi+' dBm',n.ssid)))}).catch(()=>$('ssid').innerHTML='<option value="">Scan failed – try again</option>')}
function join(){const s=$('ssidTxt').value||$('ssid').value;if(!s)return alert('Pick or type a network first');
 post('/api/wifi',{ssid:s,pass:$('pass').value}).then(()=>{$('wst').textContent='Saved. The lamp is connecting…';$('pass').value='';setTimeout(poll,4000)}).catch(()=>alert('No reply – try again'))}
draw();poll();setInterval(poll,6000);
</script></body></html>)HTML";
