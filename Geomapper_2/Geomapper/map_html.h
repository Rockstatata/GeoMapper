#pragma once
const char* MAP_HTML = R"HTML(
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
  .layer-btn{padding:8px;text-align:center;transition:all 0.2s ease;font-size:10px;line-height:1.2}
  .layer-btn.active{opacity:1;transform:scale(1)}
  .layer-btn:not(.active){opacity:0.4;transform:scale(0.95);background:#374151!important}
  .layer-btn:hover{transform:scale(1.05)}
  .preset-btn{font-size:10px;padding:4px 8px;background:#374151;border-color:#4b5563}
  .preset-btn:hover{background:#4b5563}
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
      <h4 style="margin:0 0 12px 0">📊 Sonar Layer Control</h4>
      
      <!-- Layer Toggle Buttons -->
      <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-bottom:12px">
        <button id="frontToggle" class="layer-btn active" data-layer="front" style="background:#60a5fa;border-color:#60a5fa;color:#fff">
          <div style="font-size:10px">FRONT</div>
          <div style="font-size:8px;opacity:0.8">0° base</div>
        </button>
        <button id="rightToggle" class="layer-btn active" data-layer="right" style="background:#f87171;border-color:#f87171;color:#fff">
          <div style="font-size:10px">RIGHT</div>
          <div style="font-size:8px;opacity:0.8">+90° offset</div>
        </button>
        <button id="leftToggle" class="layer-btn active" data-layer="left" style="background:#34d399;border-color:#34d399;color:#fff">
          <div style="font-size:10px">LEFT</div>
          <div style="font-size:8px;opacity:0.8">-90° offset</div>
        </button>
      </div>
      
      <!-- Quick Presets -->
      <div style="display:flex;gap:6px;margin-bottom:12px">
        <button id="showAll" class="preset-btn">All Layers</button>
        <button id="showFrontOnly" class="preset-btn">Front Only</button>
        <button id="hideAll" class="preset-btn">Hide All</button>
      </div>
      
      <!-- Legend -->
      <div class="legend" style="font-size:11px">
        <div class="legend-dot" style="background:#60a5fa"></div><span>Front Sonar Path</span>
        <div class="legend-dot" style="background:#f87171"></div><span>Right Sonar Path</span>
        <div class="legend-dot" style="background:#34d399"></div><span>Left Sonar Path</span>
        <div class="legend-dot" style="background:#a5b4fc"></div><span>Combined Outline</span>
      </div>
      
      <div class="stats">
        Each rotation: 18° CCW step<br>
        Total coverage: 360° in 20 steps<br>
        Click layers to toggle visibility
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

// Layer visibility state
let layerVisibility = {
  front: true,
  right: true,
  left: true
};

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

function drawRay(angle, dist, color, label, visible = true){
  if (dist < 5 || !visible) return; // Skip if not visible or too close
  
  const p = p2(angle, dist);
  
  // Beam line (subtle)
  ctx.strokeStyle = color + '44'; 
  ctx.lineWidth = 1;
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
  
  // Distance label (smaller and less intrusive)
  ctx.fillStyle = color + 'CC';
  ctx.font = '8px system-ui';
  const labelText = Math.round(dist) + 'cm';
  ctx.fillText(labelText, p.x + 6, p.y - 6);
  
  return p; // Return point for path drawing
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

function drawSonarPath(points, color, lineWidth = 2, alpha = 0.8) {
  if (points.length < 2) return;
  
  // Sort points by angle for proper connection
  points.sort((a, b) => {
    const angleA = Math.atan2(a.y - C.y, a.x - C.x);
    const angleB = Math.atan2(b.y - C.x, b.x - C.x);
    return angleA - angleB;
  });
  
  ctx.strokeStyle = color;
  ctx.lineWidth = lineWidth;
  ctx.globalAlpha = alpha;
  ctx.setLineDash([]);
  
  ctx.beginPath();
  ctx.moveTo(points[0].x, points[0].y);
  
  for (let i = 1; i < points.length; i++) {
    ctx.lineTo(points[i].x, points[i].y);
  }
  
  // Close the path if we have enough points
  if (points.length >= 5) {
    ctx.closePath();
  }
  
  ctx.stroke();
  ctx.globalAlpha = 1;
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
    
    // Separate point collections for each sonar
    const frontPoints = [];
    const rightPoints = [];
    const leftPoints = [];
    let maxSampleAngle = 0;
    
    // Draw individual sonar rays and collect points
    for (let i = 0; i < items.length; i++){
      const s = items[i];
      
      // Calculate angles: front = a, right = a + 90, left = a - 90
      const aF = s.a, aR = s.a + 90, aL = s.a - 90;
      maxSampleAngle = Math.max(maxSampleAngle, Math.abs(s.a));
      
      // Draw rays only for visible layers and collect points
      const frontPoint = drawRay(aF, s.f, '#60a5fa', 'F', layerVisibility.front);
      const rightPoint = drawRay(aR, s.r, '#f87171', 'R', layerVisibility.right);
      const leftPoint = drawRay(aL, s.l, '#34d399', 'L', layerVisibility.left);
      
      // Collect points for path drawing
      if (frontPoint && s.f > 5) frontPoints.push(frontPoint);
      if (rightPoint && s.r > 5) rightPoints.push(rightPoint);
      if (leftPoint && s.l > 5) leftPoints.push(leftPoint);
    }
    
    // Draw individual sonar paths
    if (layerVisibility.front && frontPoints.length >= 2) {
      drawSonarPath(frontPoints, '#60a5fa', 3, 0.9);
    }
    if (layerVisibility.right && rightPoints.length >= 2) {
      drawSonarPath(rightPoints, '#f87171', 3, 0.9);
    }
    if (layerVisibility.left && leftPoints.length >= 2) {
      drawSonarPath(leftPoints, '#34d399', 3, 0.9);
    }
    
    // Draw combined outline if multiple layers are visible
    const visibleLayers = Object.values(layerVisibility).filter(v => v).length;
    if (visibleLayers > 1) {
      const allPoints = [
        ...(layerVisibility.front ? frontPoints : []),
        ...(layerVisibility.right ? rightPoints : []),
        ...(layerVisibility.left ? leftPoints : [])
      ];
      
      if (allPoints.length >= 3) {
        drawSonarPath(allPoints, '#a5b4fc', 2, 0.5);
      }
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

// Layer control functionality
function updateLayerButton(layer, isActive) {
  const button = document.getElementById(layer + 'Toggle');
  if (isActive) {
    button.classList.add('active');
  } else {
    button.classList.remove('active');
  }
}

function toggleLayer(layer) {
  layerVisibility[layer] = !layerVisibility[layer];
  updateLayerButton(layer, layerVisibility[layer]);
}

// Add event listeners for layer toggles
document.getElementById('frontToggle').onclick = () => toggleLayer('front');
document.getElementById('rightToggle').onclick = () => toggleLayer('right');
document.getElementById('leftToggle').onclick = () => toggleLayer('left');

// Preset buttons
document.getElementById('showAll').onclick = () => {
  layerVisibility = { front: true, right: true, left: true };
  updateLayerButton('front', true);
  updateLayerButton('right', true);
  updateLayerButton('left', true);
};

document.getElementById('showFrontOnly').onclick = () => {
  layerVisibility = { front: true, right: false, left: false };
  updateLayerButton('front', true);
  updateLayerButton('right', false);
  updateLayerButton('left', false);
};

document.getElementById('hideAll').onclick = () => {
  layerVisibility = { front: false, right: false, left: false };
  updateLayerButton('front', false);
  updateLayerButton('right', false);
  updateLayerButton('left', false);
};

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