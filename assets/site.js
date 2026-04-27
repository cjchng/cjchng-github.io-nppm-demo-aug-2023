const canvas = document.querySelector("#projection-canvas");
const ctx = canvas?.getContext("2d");

function fitCanvas() {
  if (!canvas || !ctx) return;

  const ratio = window.devicePixelRatio || 1;
  const rect = canvas.getBoundingClientRect();
  canvas.width = Math.max(1, Math.floor(rect.width * ratio));
  canvas.height = Math.max(1, Math.floor(rect.height * ratio));
  ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
}

function drawProjection(time = 0) {
  if (!canvas || !ctx) return;

  const rect = canvas.getBoundingClientRect();
  const width = rect.width;
  const height = rect.height;
  const cx = width * 0.5;
  const cy = height * 0.5;
  const radius = Math.min(width, height) * 0.38;
  const spin = time * 0.00028;

  ctx.clearRect(0, 0, width, height);

  const bg = ctx.createLinearGradient(0, 0, width, height);
  bg.addColorStop(0, "#121f31");
  bg.addColorStop(0.55, "#0d1522");
  bg.addColorStop(1, "#17263b");
  ctx.fillStyle = bg;
  ctx.fillRect(0, 0, width, height);

  ctx.save();
  ctx.translate(cx, cy);

  ctx.strokeStyle = "rgba(203, 213, 225, 0.18)";
  ctx.lineWidth = 1;
  for (let ring = 1; ring <= 6; ring += 1) {
    ctx.beginPath();
    ctx.arc(0, 0, (radius * ring) / 6, 0, Math.PI * 2);
    ctx.stroke();
  }

  for (let spoke = 0; spoke < 24; spoke += 1) {
    const angle = (Math.PI * 2 * spoke) / 24 + spin;
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(Math.cos(angle) * radius, Math.sin(angle) * radius);
    ctx.stroke();
  }

  const sweep = ctx.createRadialGradient(0, 0, radius * 0.1, 0, 0, radius);
  sweep.addColorStop(0, "rgba(69, 212, 131, 0.38)");
  sweep.addColorStop(0.62, "rgba(36, 90, 159, 0.18)");
  sweep.addColorStop(1, "rgba(226, 169, 59, 0.05)");
  ctx.fillStyle = sweep;
  ctx.beginPath();
  ctx.arc(0, 0, radius, 0, Math.PI * 2);
  ctx.fill();

  const points = [
    [-0.62, -0.44, "#e2a93b"],
    [-0.28, 0.42, "#45d483"],
    [0.2, -0.2, "#78a7ff"],
    [0.58, 0.3, "#e86661"],
  ];

  points.forEach(([px, py, color], index) => {
    const pulse = 1 + Math.sin(time * 0.003 + index) * 0.12;
    const x = px * radius;
    const y = py * radius;

    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.arc(x, y, 7 * pulse, 0, Math.PI * 2);
    ctx.fill();

    ctx.strokeStyle = "rgba(255, 255, 255, 0.34)";
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(x, y, 15 * pulse, 0, Math.PI * 2);
    ctx.stroke();
  });

  ctx.strokeStyle = "rgba(255, 255, 255, 0.74)";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.arc(0, 0, radius, 0, Math.PI * 2);
  ctx.stroke();

  ctx.restore();

  ctx.fillStyle = "rgba(255, 255, 255, 0.76)";
  ctx.font = "700 13px SFMono-Regular, Consolas, monospace";
  ctx.fillText("CJ_NPPM_META/defaultS/active_recipes.yaml", 18, height - 22);

  requestAnimationFrame(drawProjection);
}

if (canvas && ctx) {
  fitCanvas();
  window.addEventListener("resize", fitCanvas);
  requestAnimationFrame(drawProjection);
}
