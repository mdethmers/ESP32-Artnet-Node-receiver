#ifndef WEBASSETS_H
#define WEBASSETS_H

// Stylesheet and script for the configuration UI, served from /style.css and
// /app.js. They live in a header rather than the sketch because the Arduino
// builder scans .ino files with ctags to generate prototypes, and ctags does
// not understand C++11 raw string literals — it reads the JavaScript inside
// them as C++ and emits prototypes for it, which then fail to compile.

#define ASSET_VER "1"

// Colours follow the node's own signal language: amber is anything you can act
// on (the status LED and a venue worklight are both amber), green is live,
// blue is AP mode, red is trouble. Numbers are always monospace and tabular so
// universes and addresses line up the way they do on a patch sheet.
const char CSS_STYLES[] PROGMEM = R"css(
:root{
  --canvas:#eceff3; --panel:#fff; --sunk:#f4f6f9;
  --ink:#141821; --muted:#5b6472; --line:#d8dde5;
  --accent:#9c580c; --lit:#f0a73c; --lit-ink:#2a1a05;
  --live:#2c8f4f; --warn:#bf3a2e; --ap:#3767cf;
  --shadow:0 1px 2px rgba(18,22,30,.07),0 8px 22px rgba(18,22,30,.05);
  --f-ui:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif;
  --f-mono:ui-monospace,"Cascadia Mono",SFMono-Regular,"SF Mono",Consolas,"Liberation Mono",monospace;
}
:root[data-theme=dark]{
  --canvas:#101319; --panel:#191d25; --sunk:#20242e;
  --ink:#e7eaf0; --muted:#8b94a3; --line:#2b313c;
  --accent:#f0a73c; --lit:#f0a73c; --lit-ink:#1a1206;
  --live:#4ed489; --warn:#f2786e; --ap:#7ba6ff;
  --shadow:0 1px 2px rgba(0,0,0,.45);
}
@media (prefers-color-scheme:dark){:root:not([data-theme=light]){
  --canvas:#101319; --panel:#191d25; --sunk:#20242e;
  --ink:#e7eaf0; --muted:#8b94a3; --line:#2b313c;
  --accent:#f0a73c; --lit:#f0a73c; --lit-ink:#1a1206;
  --live:#4ed489; --warn:#f2786e; --ap:#7ba6ff;
  --shadow:0 1px 2px rgba(0,0,0,.45);
}}
*,*::before,*::after{box-sizing:border-box}
[hidden]{display:none!important}
html{-webkit-text-size-adjust:100%}
body{margin:0;padding:0 15px 36px;background:var(--canvas);color:var(--ink);
  font:400 15px/1.5 var(--f-ui);-webkit-font-smoothing:antialiased}
.shell{max-width:720px;margin:0 auto}
:focus-visible{outline:2px solid var(--lit);outline-offset:2px;border-radius:4px}

/* Masthead: who am I, where am I, what am I speaking */
.masthead{display:flex;align-items:flex-start;gap:12px;padding:26px 2px 18px}
.masthead .id{flex:1 1 auto;min-width:0}
h1{margin:0;font:700 clamp(21px,5vw,27px)/1.15 var(--f-ui);letter-spacing:-.015em;
  word-break:break-word}
.readout{margin:6px 0 0;font:400 13px/1.6 var(--f-mono);color:var(--muted)}
.dot{flex:none;width:9px;height:9px;margin-top:11px;border-radius:50%;background:var(--live)}
.dot.ap{background:var(--ap)}

/* Panels read like rack units: silkscreened legend, then controls */
.panel{margin:0 0 14px;background:var(--panel);border:1px solid var(--line);
  border-radius:10px;box-shadow:var(--shadow)}
.legend{position:relative;display:flex;align-items:center;gap:9px;padding:13px 16px;
  border-bottom:1px solid var(--line);font:600 11px/1 var(--f-ui);
  letter-spacing:.13em;text-transform:uppercase;color:var(--muted)}
.foot{margin:0;padding:12px 16px;border-top:1px solid var(--line);
  font:400 12.5px/1.5 var(--f-ui);color:var(--muted)}

/* min-height keeps the rhythm even: a toggle or slider row is shorter than a
   text-input row, and without it the panels read as ragged. */
.row{position:relative;display:grid;grid-template-columns:1fr 1.1fr;gap:9px 18px;
  align-items:center;min-height:64px;padding:13px 16px;border-bottom:1px solid var(--line)}
.row:last-child,.panel>:last-child>.row:last-child{border-bottom:0}
.row:hover,.row:focus-within{z-index:4}
.lbl{display:flex;align-items:center;gap:6px;min-width:0;font-weight:500}
.hint{grid-column:1/-1;margin:-1px 0 0;font:400 12.5px/1.5 var(--f-ui);color:var(--muted)}
@media (max-width:560px){.row{grid-template-columns:1fr}}

input[type=text],input[type=password],input[type=number]{width:100%;padding:9px 11px;
  border:1px solid var(--line);border-radius:7px;background:var(--sunk);color:var(--ink);
  font:400 15px/1.35 var(--f-ui)}
input[type=number],.mono{font-family:var(--f-mono);font-variant-numeric:tabular-nums}
input::placeholder{color:var(--muted);opacity:.75}
input[type=file]{width:100%;padding:8px;border:1px dashed var(--line);border-radius:7px;
  background:var(--sunk);color:var(--muted);font:400 13.5px var(--f-ui);cursor:pointer}
input[type=file]::file-selector-button{margin-right:10px;padding:7px 12px;border:0;
  border-radius:6px;background:var(--lit);color:var(--lit-ink);cursor:pointer;
  font:600 13px var(--f-ui)}
input[type=color]{width:100%;max-width:130px;height:38px;padding:3px;
  border:1px solid var(--line);border-radius:7px;background:var(--sunk);cursor:pointer}
input[type=range]{width:100%;height:24px;accent-color:var(--lit)}
.meter{display:flex;align-items:center;gap:12px}
.meter output{flex:none;min-width:3.6em;text-align:right;
  font:600 14px var(--f-mono);font-variant-numeric:tabular-nums}
.meter output.wide{min-width:7.6em}

/* One segmented control for every either/or choice on the page */
.seg{display:flex;flex-wrap:wrap;gap:3px;padding:3px;background:var(--sunk);
  border:1px solid var(--line);border-radius:8px}
.seg input{position:absolute;width:1px;height:1px;margin:0;opacity:0}
.seg label{flex:1 1 auto;padding:7px 10px;border-radius:6px;text-align:center;
  cursor:pointer;white-space:nowrap;font:500 13.5px/1.25 var(--f-ui);color:var(--muted);
  -webkit-user-select:none;user-select:none}
.seg label:hover{color:var(--ink)}
.seg input:checked+label{background:var(--lit);color:var(--lit-ink);font-weight:600}
.seg input:focus-visible+label{outline:2px solid var(--lit);outline-offset:2px}

.sw{position:relative;display:inline-block;flex:none;width:42px;height:24px}
.sw input{position:absolute;z-index:2;width:100%;height:100%;margin:0;opacity:0;cursor:pointer}
.sw i{position:absolute;inset:0;border-radius:99px;background:var(--line);transition:background .15s}
.sw i::after{content:'';position:absolute;top:3px;left:3px;width:18px;height:18px;
  border-radius:50%;background:var(--panel);box-shadow:0 1px 2px rgba(0,0,0,.35);
  transition:transform .15s}
.sw input:checked+i{background:var(--lit)}
.sw input:checked+i::after{transform:translateX(18px)}
.sw input:focus-visible+i{outline:2px solid var(--lit);outline-offset:2px}

/* Tooltips anchor to the row, not the icon, so they never clip off-screen */
.tip{display:inline-flex;flex:none;align-items:center;justify-content:center;
  width:16px;height:16px;padding:0;border:1px solid var(--line);border-radius:50%;
  background:none;color:var(--muted);font:700 10px/1 var(--f-ui);cursor:pointer}
.tip:hover{color:var(--lit);border-color:var(--lit)}
.tip::after{content:attr(data-tip);position:absolute;left:16px;right:16px;
  bottom:calc(100% - 6px);z-index:20;padding:9px 11px;border-radius:8px;
  background:var(--ink);color:var(--panel);text-align:left;letter-spacing:0;
  text-transform:none;font:400 12.5px/1.5 var(--f-ui);box-shadow:0 8px 24px rgba(0,0,0,.3);
  opacity:0;visibility:hidden;transition:opacity .12s;pointer-events:none}
.tip:hover::after,.tip.open::after{opacity:1;visibility:visible}

/* The universe map. Each output is a boxed group holding the universes it
   listens on, so "these two belong to output 2" is drawn rather than implied
   by a slightly wider gap. The U prefix says what the numbers are. */
.patch{padding:16px}
.patchhead{margin:0 0 11px;color:var(--muted);font:600 11px/1 var(--f-ui);
  letter-spacing:.13em;text-transform:uppercase}
/* Fixed column counts rather than flex wrapping, so four outputs never break
   into an uneven 3 + 1. */
.strip{display:grid;grid-template-columns:repeat(4,1fr);gap:10px}
@media (max-width:700px){.strip{grid-template-columns:repeat(2,1fr)}}
@media (max-width:430px){.strip{grid-template-columns:1fr}}
.grp{min-width:0;border:1px solid var(--line);border-radius:8px;
  background:var(--sunk);overflow:hidden}
.grp b{display:block;padding:6px 8px;border-bottom:1px solid var(--line);
  background:var(--panel);text-align:center;color:var(--muted);
  font:600 10px/1.2 var(--f-ui);letter-spacing:.11em;text-transform:uppercase}
/* Column count and type size are set inline from the universes-per-output
   value: a group is always one even row, and the chips shrink their text
   rather than clipping it when a high start universe makes the labels long. */
.cells{display:grid;gap:4px;padding:8px;font:600 12.5px/1 var(--f-mono)}
.u{min-width:0;height:30px;display:flex;align-items:center;justify-content:center;
  border-radius:4px;background:var(--lit);color:var(--lit-ink);overflow:hidden;
  font:inherit;font-variant-numeric:tabular-nums}
.tally{margin:14px 0 0;font:400 13px/1.55 var(--f-mono);color:var(--muted)}
.tally b{color:var(--ink);font-weight:600}

.nodes{margin:0;padding:0;list-style:none;max-height:238px;overflow-y:auto}
.nodes li{display:flex;align-items:center;gap:10px;padding:11px 16px;
  border-bottom:1px solid var(--line)}
.nodes li:last-child{border-bottom:0}
.nodes .nm{flex:1 1 auto;min-width:0;overflow:hidden;text-overflow:ellipsis;
  white-space:nowrap;font-weight:500;color:var(--ink);text-decoration:none}
.nodes a.nm:hover{color:var(--accent);text-decoration:underline}
.nodes .ip{flex:none;font:400 13px var(--f-mono);color:var(--muted)}
.here{background:var(--sunk)}
.tag{flex:none;padding:4px 7px;border:1px solid currentColor;border-radius:99px;
  color:var(--accent);font:600 9.5px/1 var(--f-ui);letter-spacing:.11em;text-transform:uppercase}

.icon{display:grid;place-items:center;flex:none;width:30px;height:30px;padding:0;
  border:1px solid var(--line);border-radius:7px;background:var(--panel);
  color:var(--muted);cursor:pointer}
.legend .icon{margin-left:auto}
.icon.lg{width:36px;height:36px}
.icon:hover{color:var(--lit);border-color:var(--lit)}
.icon svg{width:15px;height:15px;fill:none;stroke:currentColor;stroke-width:2;
  stroke-linecap:round;stroke-linejoin:round}
.icon.lg svg{width:17px;height:17px}
.icon.busy svg{animation:spin .8s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}

details>summary{position:relative;display:flex;align-items:center;gap:9px;
  padding:13px 16px;cursor:pointer;list-style:none;color:var(--muted);
  font:600 11px/1 var(--f-ui);letter-spacing:.13em;text-transform:uppercase}
details>summary::-webkit-details-marker{display:none}
details>summary::before{content:'';width:6px;height:6px;margin-right:1px;
  border-right:1.5px solid currentColor;border-bottom:1.5px solid currentColor;
  transform:rotate(-45deg);transition:transform .15s}
details[open]>summary::before{transform:rotate(45deg)}
details[open]>summary{border-bottom:1px solid var(--line)}
.pins{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));
  gap:11px;padding:14px 16px}
.pin{display:flex;align-items:center;gap:9px}
.pin span{flex:none;color:var(--muted);font:600 10px/1 var(--f-ui);
  letter-spacing:.11em;text-transform:uppercase}
.pin input{width:66px;text-align:center}

.dock{position:sticky;bottom:0;z-index:6;display:flex;align-items:center;gap:12px;
  margin:18px 0 0;padding:12px;background:var(--panel);border:1px solid var(--line);
  border-radius:10px;box-shadow:0 -2px 18px rgba(18,22,30,.13)}
.dock .note{flex:1 1 auto;min-width:0;font:400 12.5px/1.4 var(--f-ui);color:var(--muted)}
.btn{display:inline-block;padding:11px 18px;border:1px solid var(--line);border-radius:8px;
  background:var(--sunk);color:var(--ink);text-decoration:none;cursor:pointer;
  white-space:nowrap;font:600 14.5px var(--f-ui)}
.btn:hover{border-color:var(--muted)}
.btn.go{background:var(--lit);border-color:var(--lit);color:var(--lit-ink)}
.btn.go:hover{filter:brightness(1.07)}
.btn[disabled]{opacity:.6;cursor:default}
.btn.link{padding:6px 10px;font-size:12.5px}
.note b{color:var(--warn)}
.pagefoot{display:flex;align-items:center;gap:12px;padding:20px 2px 0;
  font:400 12.5px/1.5 var(--f-ui);color:var(--muted)}
.pagefoot a{color:var(--accent)}
@media (prefers-reduced-motion:reduce){
  *{animation-duration:.01ms!important;animation-iteration-count:1!important;
    transition-duration:.01ms!important}
}
)css";

const char JS_CODE[] PROGMEM = R"js(
(function(){
var D=document;
function $(s){return D.querySelector(s)}
function $$(s){return Array.prototype.slice.call(D.querySelectorAll(s))}
function show(s,on){var e=$(s);if(e)e.hidden=!on}
function pick(n){var e=D.querySelector('input[name='+n+']:checked');return e?parseInt(e.value,10):0}

var SUN='<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="4"/><path d="M12 2.5v2M12 19.5v2M4.6 4.6 6 6M18 18l1.4 1.4M2.5 12h2M19.5 12h2M4.6 19.4 6 18M18 6l1.4-1.4"/></svg>';
var MOON='<svg viewBox="0 0 24 24"><path d="M20.5 14.3A8.5 8.5 0 0 1 9.7 3.5a8.5 8.5 0 1 0 10.8 10.8z"/></svg>';
var ARROWS='<svg viewBox="0 0 24 24"><path d="M20.5 12a8.5 8.5 0 1 1-2.5-6"/><path d="M20.5 3.5V8H16"/></svg>';

function isDark(){var t=D.documentElement.getAttribute('data-theme');
  return t?t==='dark':matchMedia('(prefers-color-scheme:dark)').matches}
function paintTheme(){var b=$('#theme');if(!b)return;
  var d=isDark(),t=d?'Switch to light mode':'Switch to dark mode';
  b.innerHTML=d?SUN:MOON;b.title=t;b.setAttribute('aria-label',t)}

function esc(s){return String(s).replace(/[&<>"]/g,function(m){
  return m==='&'?'&amp;':m==='<'?'&lt;':m==='>'?'&gt;':'&quot;'})}
function ago(s){return s<3?'just now':s<60?s+'s ago':Math.round(s/60)+' min ago'}
function drawNodes(d){
  var h='<li class="here"><span class="nm">'+esc(d.self.n)+'</span>'+
        '<span class="ip">'+esc(d.self.ip)+'</span><span class="tag">This node</span></li>';
  d.nodes.forEach(function(n){
    h+='<li><a class="nm" href="http://'+esc(n.ip)+'/">'+(n.n?esc(n.n):'Unnamed node')+'</a>'+
       '<span class="ip">'+esc(n.ip)+'</span><span class="ip">'+ago(n.a)+'</span></li>'});
  $('#nodes').innerHTML=h;
  $('#nodecount').textContent=d.nodes.length
    ?d.nodes.length+(d.nodes.length>1?' other nodes are':' other node is')+' answering polls. Select one to open its configuration.'
    :'No other Art-Net nodes have answered. Refresh to poll the network again.'}
function wait(ms){return new Promise(function(r){setTimeout(r,ms)})}
function refresh(){
  var b=$('#reload');if(!b||b.classList.contains('busy'))return;
  b.classList.add('busy');
  fetch('/nodes?poll=1').then(function(r){return r.json()}).then(drawNodes)
    .then(function(){return wait(1400)})
    .then(function(){return fetch('/nodes')})
    .then(function(r){return r.json()}).then(drawNodes)
    .catch(function(){$('#nodecount').textContent=
      'The node did not answer. Check the connection and try again.'})
    .then(function(){b.classList.remove('busy')})}

function chMode(){return $('#lt1').checked?0:pick('channelMode')}
function perUniverse(){var m=chMode();return m===2?102:m===1?128:170}
function isSACN(){return pick('protocolMode')===1}

/* Everything the form shows is derived here, so any change re-runs one pass. */
function syncAll(){
  var apa=$('#lt1').checked,cm=chMode(),per=perUniverse();
  var upo=+$('#uniSlider').value,outs=+$('#outSlider').value;
  var raw=parseInt($('#startuniverse').value,10)||0,base=raw+(isSACN()?1:0);

  show('#chRow',!apa);show('#apaNote',apa);
  if(apa)$('#cm0').checked=true;

  /* Widest label in the map decides the chip type size, so a start universe of
     1000 stays readable in a four-across group instead of being cut off. */
  var digits=String(base+outs*upo-1).length;
  var size=upo<3?12.5:(digits>3?10.5:(upo>3?11.5:12));
  var h='';
  for(var o=0;o<outs;o++){var u='';
    for(var i=0;i<upo;i++)u+='<span class="u">U'+(base+o*upo+i)+'</span>';
    h+='<div class="grp"><b>Output '+(o+1)+'</b><div class="cells" style="grid-template-columns:repeat('+
       upo+',1fr);font-size:'+size+'px">'+u+'</div></div>'}
  $('#strip').innerHTML=h;
  $('#tally').innerHTML='<b>'+(upo*per)+'</b> pixels per output · <b>'+(outs*upo)+
    '</b> universes · <b>'+(outs*upo*per)+'</b> pixels in total';
  $('#numledsoutput').value=upo*per;
  $('#numoutput').value=outs;
  $('#uniVal').value=upo+(upo>1?' universes':' universe');
  $('#outVal').value=outs;
  $('#uniHint').textContent=isSACN()
    ?'sACN counts universes from 1. Enter the number your console shows.'
    :'Art-Net counts universes from 0. Enter the number your console shows.';

  var m=pick('outputMode');
  show('#colorRow',m===1);
  show('#w1Row',m===1&&cm>=1&&!apa);
  show('#w2Row',m===1&&cm===2&&!apa);
  $('#modeNote').textContent=['Strips follow incoming DMX. Use this for normal running.',
    'Strips hold one colour and ignore DMX.',
    'Strips run a red-green-blue check pattern and ignore DMX.',
    'Strips fade through colours and ignore DMX.'][m]||'';

  show('#wifiRows',pick('link')===1);
  show('#ipRows',$('#useStaticIP').checked);
  show('#pinRows',$('#enableCustomPins').checked);

  $('#brVal').value=$('#ledbrightness').value;
  $('#w1Val').value=$('#whiteLevel').value;
  $('#w2Val').value=$('#white2Level').value}

/* The field shows what the console shows; the form posts the raw 0-based value. */
function fromField(){
  var v=parseInt($('#uniStart').value,10);if(isNaN(v))return;
  $('#startuniverse').value=Math.max(0,isSACN()?v-1:v);syncAll()}
function toField(){
  var raw=parseInt($('#startuniverse').value,10)||0;
  $('#uniStart').value=raw+(isSACN()?1:0);syncAll()}

D.addEventListener('DOMContentLoaded',function(){
  paintTheme();
  var tb=$('#theme');
  if(tb)tb.addEventListener('click',function(){
    var d=!isDark();
    D.documentElement.setAttribute('data-theme',d?'dark':'light');
    try{localStorage.setItem('theme',d?'dark':'light')}catch(e){}
    paintTheme()});

  var rb=$('#reload');
  if(rb){rb.innerHTML=ARROWS;rb.addEventListener('click',refresh)}

  var f=$('#cfg');
  if(f){
    f.addEventListener('input',function(e){
      if(e.target.id==='uniStart')fromField();else syncAll()});
    f.addEventListener('change',function(e){
      if(e.target.name==='protocolMode')toField();else syncAll()});
    f.addEventListener('submit',function(){
      var b=$('#save');if(b){b.disabled=true;b.value='Saving…'}});
    syncAll()}

  var pw=$('#pwshow');
  if(pw)pw.addEventListener('click',function(){
    var i=$('#password'),hidden=i.type==='password';
    i.type=hidden?'text':'password';pw.textContent=hidden?'Hide':'Show'});

  D.addEventListener('click',function(e){
    var t=e.target&&e.target.closest?e.target.closest('.tip'):null;
    $$('.tip.open').forEach(function(x){if(x!==t)x.classList.remove('open')});
    if(t)t.classList.toggle('open')});
  D.addEventListener('keydown',function(e){
    if(e.key==='Escape')$$('.tip.open').forEach(function(x){x.classList.remove('open')})})});
})();
)js";

#endif
