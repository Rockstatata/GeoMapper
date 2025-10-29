#pragma once
const char* MAP_HTML PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
<meta charset="utf-8"/>
<title>GeoMapper – Advanced Radar Sweep</title>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<style>
  body{margin:0;background:#0b1220;color:#e5edff;font-family:system-ui,Segoe UI,Roboto,Arial}
  header{display:flex;align-items:center;gap:12px;padding:12px 16px;border-bottom:1px solid #1b2340}
  .pill{padding:4px 10px;border:1px solid #2a365e;border-radius:999px;font-size:12px}
  main{display:grid;grid-template-columns:1fr 380px;gap:16px;padding:16px}
  canvas{background:#0f1730;border:1px solid #1b2340;border-radius:12px}
  .card{background:#0f1730;border:1px solid #1b2340;border-radius:12px;padding:12px}
  .row{display:flex;justify-content:space-between;margin:6px 0}
  button{background:#1b2340;color:#e5edff;border:1px solid #2a365e;border-radius:8px;padding:8px 12px;cursor:pointer}
  button:hover{background:#22305a}
  .legend{display:grid;grid-template-columns:auto 1fr;gap:8px;align-items:center;font-size:12px}
  .legend-dot{width:12px;height:12px;border-radius:50%}
  .stats{font-size:11px;color:#94a3b8;margin-top:8px}
</style>
</head>
<body>
<header>
  <div style="font-weight:700">🎯 GeoMapper Radar</div>
  <div class="pill" id="statePill">mapping: -</div>
  <div class="pill">18° precision</div>
  <div class="pill">host: geomapper.local</div>
  <button id="refreshBtn">Refresh</button>
</header>

<main>
  <div><canvas id="cv" width="700" height="700"></canvas></div>
  <div>
    <div class="card">
      <div class="row"><span>🔄 Rotation Steps</span><span id="count">0/20</span></div>
      <div class="row"><span>📡 Total Points</span><span id="totalPoints">0</span></div>
      <div class="row"><span>✅ Complete</span><span id="done">no</span></div>
      <div class="row"><span>🎯 Current Angle</span><span id="currentAngle">0°</span></div>
    </div>
    
    <div class="card" style="margin-top:12px">
      <h4 style="margin:0 0 8px 0">📊 Sonar Legend</h4>
      <div class="legend">
        <div class="legend-dot" style="background:#60a5fa"></div><span>Front Sonar (0° base)</span>
        <div class="legend-dot" style="background:#f87171"></div><span>Right Sonar (+90° offset)</span>
        <div class="legend-dot" style="background:#34d399"></div><span>Left Sonar (-90° offset)</span>
        <div class="legend-dot" style="background:#a5b4fc"></div><span>Connected Path</span>
      </div>
      <div class="stats">
        Each rotation: 18° CCW step<br>
        Total coverage: 360° in 20 steps<br>
        Timing: 250ms rotate + 1s measure
      </div>
    </div>
    
    <div class="card" style="margin-top:12px">
      <div style="font-size:13px;line-height:1.4">
        <div>🚗 <strong>Controls:</strong></div>
        <div style="margin:4px 0">Send <code style="background:#1b2340;padding:2px 4px;border-radius:4px">'X'</code> via Bluetooth to start/stop mapping</div>
        <div style="margin:4px 0">Car rotates while stationary for precision</div>
        <div style="margin:4px 0">Real-time radar sweep visualization</div>
      </div>
    </div>
  </div>
</main>

<script>
const cv = document.getElementById('cv');
const ctx = cv.getContext('2d');
const W = cv.width, H = cv.height;
const C = {x: W/2, y: H/2};
const CM = 3.5;      // px per cm (slightly smaller for better view)
const MAX = 180;     // max cm drawn (adjusted for 18° precision)
const PILL = document.getElementById('statePill');
const CNT  = document.getElementById('count');
const DONE = document.getElementById('done');
const TOTAL = document.getElementById('totalPoints');
const ANGLE = document.getElementById('currentAngle');

let animationFrame = 0;
let lastDataHash = '';

// Enhanced radar sweep animation
let sweepAngle = 0;
let sweepSpeed = 2; // degrees per frame

function drawGrid(){
  ctx.fillStyle = '#0f1730'; 
  ctx.fillRect(0,0,W,H);
  
  // Concentric range circles
  ctx.strokeStyle = '#1b2340'; 
  ctx.lineWidth = 1;
  for (let r = 25; r <= MAX; r += 25) {
    ctx.beginPath(); 
    ctx.arc(C.x, C.y, r*CM, 0, Math.PI*2); 
    ctx.stroke();
    // Range labels
    ctx.fillStyle = '#374151';
    ctx.font = '10px system-ui';
    ctx.fillText(r + 'cm', C.x + r*CM - 15, C.y - 5);
  }
  
  // Angular grid lines (every 30°)
  ctx.strokeStyle = '#1a202c';
  ctx.lineWidth = 0.5;
  for (let angle = 0; angle < 360; angle += 30) {
    const rad = angle * Math.PI/180;
    const x2 = C.x + MAX*CM * Math.cos(rad);
    const y2 = C.y - MAX*CM * Math.sin(rad);
    ctx.beginPath();
    ctx.moveTo(C.x, C.y);
    ctx.lineTo(x2, y2);
    ctx.stroke();
    
    // Angle labels
    ctx.fillStyle = '#374151';
    ctx.font = '10px system-ui';
    const labelX = C.x + (MAX*CM + 15) * Math.cos(rad);
    const labelY = C.y - (MAX*CM + 15) * Math.sin(rad);
    ctx.fillText(angle + '°', labelX - 8, labelY + 3);
  }
  
  // Car representation
  ctx.fillStyle = '#8ab4ff';
  ctx.strokeStyle = '#60a5fa';
  ctx.lineWidth = 2;
  ctx.beginPath(); 
  ctx.arc(C.x, C.y, 12, 0, Math.PI*2); 
  ctx.fill();
  ctx.stroke();
  
  // Car direction indicator (always points to current 0°)
  ctx.strokeStyle = '#60a5fa';
  ctx.lineWidth = 3;
  ctx.beginPath();
  ctx.moveTo(C.x, C.y);
  ctx.lineTo(C.x + 20, C.y);
  ctx.stroke();
  
  ctx.fillStyle = '#e5edff';
  ctx.font = 'bold 11px system-ui';
  ctx.fillText('CAR', C.x + 18, C.y - 18);
}

function p2(angleDeg, dist){
  const a = angleDeg * Math.PI/180;
  const r = Math.min(dist, MAX) * CM;
  return {x: C.x + r*Math.cos(a), y: C.y - r*Math.sin(a)};
}

function drawRay(angle, dist, color, label){
  if (dist < 5) return; // Skip very close readings (likely noise)
  
  const p = p2(angle, dist);
  
  // Beam line
  ctx.strokeStyle = color + '66'; 
  ctx.lineWidth = 1.5;
  ctx.beginPath(); 
  ctx.moveTo(C.x, C.y); 
  ctx.lineTo(p.x, p.y); 
  ctx.stroke();
  
  // Detection point
  ctx.fillStyle = color;
  ctx.strokeStyle = color;
  ctx.lineWidth = 2;
  ctx.beginPath(); 
  ctx.arc(p.x, p.y, 4, 0, Math.PI*2); 
  ctx.fill();
  ctx.stroke();
  
  // Distance label
  ctx.fillStyle = color;
  ctx.font = '9px system-ui';
  const labelText = Math.round(dist) + 'cm';
  ctx.fillText(labelText, p.x + 6, p.y - 6);
}

function drawSweepRadar(currentAngle, isActive) {
  if (!isActive) return;
  
  // Animated sweep effect
  sweepAngle += sweepSpeed;
  if (sweepAngle > 360) sweepAngle = 0;
  
  // Primary sweep line
  const sweepRad = (currentAngle + sweepAngle * 0.1) * Math.PI/180;
  const sweepX = C.x + MAX*CM * Math.cos(sweepRad);
  const sweepY = C.y - MAX*CM * Math.sin(sweepRad);
  
  // Gradient sweep effect
  const gradient = ctx.createRadialGradient(C.x, C.y, 0, C.x, C.y, MAX*CM);
  gradient.addColorStop(0, '#93c5fd44');
  gradient.addColorStop(0.7, '#60a5fa22');
  gradient.addColorStop(1, '#60a5fa00');
  
  ctx.fillStyle = gradient;
  ctx.beginPath();
  ctx.moveTo(C.x, C.y);
  ctx.arc(C.x, C.y, MAX*CM, sweepRad - 0.3, sweepRad + 0.3);
  ctx.closePath();
  ctx.fill();
  
  // Sweep line
  ctx.strokeStyle = '#93c5fd88';
  ctx.lineWidth = 3;
  ctx.beginPath();
  ctx.moveTo(C.x, C.y);
  ctx.lineTo(sweepX, sweepY);
  ctx.stroke();
}

function intelligentPathfinding(points){
  if (points.length < 3) return [];
  
  // Sort points by angle for logical connection
  points.sort((a, b) => {
    const angleA = Math.atan2(a.y - C.y, a.x - C.x);
    const angleB = Math.atan2(b.y - C.y, b.x - C.x);
    return angleA - angleB;
  });
  
  // Connect in angular order
  return points.map((_, i) => i);
}

async function loadAndRender(){
  try {
    const res = await fetch('/map_sweep').then(r=>r.json());
    const dataHash = JSON.stringify(res);
    
    // Update UI
    PILL.textContent = 'mapping: ' + (res.enabled ? '🔴 ACTIVE' : '⚪ idle');
    DONE.textContent = res.done ? '✅ yes' : '⏳ no';
    CNT.textContent = (res.count || 0) + '/20';
    
    const items = res.items || [];
    const totalPoints = items.length * 3; // 3 sonars per sample
    TOTAL.textContent = totalPoints;
    
    // Calculate current angle based on latest sample
    if (items.length > 0) {
      const lastItem = items[items.length - 1];
      ANGLE.textContent = Math.round(lastItem.a) + '°';
    }

    drawGrid();
    
    // Collect all detection points
    const allPoints = [];
    let maxSampleAngle = 0;
    
    for (let i = 0; i < items.length; i++){
      const s = items[i];
      
      // Calculate angles: front = a, right = a + 90, left = a - 90
      const aF = s.a, aR = s.a + 90, aL = s.a - 90;
      maxSampleAngle = Math.max(maxSampleAngle, Math.abs(s.a));
      
      // Draw sonar rays with proper colors and labels
      drawRay(aF, s.f, '#60a5fa', 'F');  // Front: Blue
      drawRay(aR, s.r, '#f87171', 'R');  // Right: Red  
      drawRay(aL, s.l, '#34d399', 'L');  // Left: Green
      
      // Add points for path calculation
      if (s.f > 5) allPoints.push(p2(aF, s.f));
      if (s.r > 5) allPoints.push(p2(aR, s.r));
      if (s.l > 5) allPoints.push(p2(aL, s.l));
    }
    
    // Draw intelligent connecting path
    if (allPoints.length >= 3){
      const pathOrder = intelligentPathfinding(allPoints);
      ctx.strokeStyle = '#a5b4fc';
      ctx.lineWidth = 2;
      ctx.globalAlpha = 0.7;
      ctx.setLineDash([5, 3]);
      
      ctx.beginPath();
      const startPoint = allPoints[pathOrder[0]];
      ctx.moveTo(startPoint.x, startPoint.y);
      
      for (let i = 1; i < pathOrder.length; i++){
        const point = allPoints[pathOrder[i]];
        ctx.lineTo(point.x, point.y);
      }
      // Close the path if we have enough points
      if (res.done && pathOrder.length > 3) {
        ctx.closePath();
      }
      
      ctx.stroke();
      ctx.setLineDash([]);
      ctx.globalAlpha = 1;
    }
    
    // Animated radar sweep
    drawSweepRadar(maxSampleAngle, res.enabled);
    
    // Progress indicator
    if (res.enabled && items.length > 0) {
      const progress = items.length / 20;
      ctx.strokeStyle = '#10b981';
      ctx.lineWidth = 6;
      ctx.beginPath();
      ctx.arc(C.x, C.y, MAX*CM + 10, -Math.PI/2, -Math.PI/2 + (progress * 2 * Math.PI));
      ctx.stroke();
    }
    
  } catch(e) {
    console.error('Failed to load mapping data:', e);
  }
  
  // Continue animation
  requestAnimationFrame(loadAndRender);
}

document.getElementById('refreshBtn').onclick = () => location.reload();

// Start the rendering loop
loadAndRender();

// Also update every 500ms for data changes
setInterval(() => {
  // Data updates are handled in the animation loop
}, 500);
</script>
</body>
</html>
)HTML";
