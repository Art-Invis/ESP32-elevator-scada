// ==== Normalize + Moving Average ====
const p = msg.payload;

const toInt = v => (v===true || v==='true' || v===1 || v==='1') ? 1 : 0;
const qualityMap = { "EXCELLENT": 1, "GOOD": 2, "MODERATE": 3, "POOR": 4, "DANGEROUS": 5 };

context.state = context.state || {
    co2: 0,
    motion_per_min: 0,
    quality_level: 1,
    fan_running: 0,
    light_running: 0,
    system_enabled: 1,
    auto_mode: 1,
    threshold: 300,
    fan_cycles: 0,
    fan_on_time: 0
};

context.co2_window = context.co2_window || [];
context.motion_window = context.motion_window || [];
const windowSize = 5;

if (p.co2 !== undefined) {
    let val = Number(p.co2);
    context.state.co2 = val; 
    context.co2_window.push(val);
    if(context.co2_window.length > windowSize) context.co2_window.shift();
}

if (p.motion_per_min !== undefined) {
    let val = Number(p.motion_per_min);
    context.state.motion_per_min = val;
    
    context.motion_window.push(val);
    if(context.motion_window.length > windowSize) context.motion_window.shift();
}

if (p.fan_running !== undefined) context.state.fan_running = toInt(p.fan_running);
if (p.light_running !== undefined) context.state.light_running = toInt(p.light_running);
if (p.system_enabled !== undefined) context.state.system_enabled = toInt(p.system_enabled);
if (p.auto_mode !== undefined) context.state.auto_mode = toInt(p.auto_mode);
if (p.threshold !== undefined) context.state.threshold = Number(p.threshold);
if (p.fan_cycles !== undefined) context.state.fan_cycles = Number(p.fan_cycles);
if (p.fan_on_time !== undefined) context.state.fan_on_time = Number(p.fan_on_time);
if (p.quality_level !== undefined) {
    let q = 0;
    if (!isNaN(p.quality_level)) {
        q = Number(p.quality_level);
    } else {
        q = qualityMap[p.quality_level] || context.state.quality_level;
    }
    context.state.quality_level = q;
}

const calcAvg = (arr) => (arr.length > 0) ? (arr.reduce((a,b)=>a+b,0)/arr.length) : 0;
const avgCO2 = calcAvg(context.co2_window) || context.state.co2; 
const avgMotion = calcAvg(context.motion_window) || context.state.motion_per_min;

msg.payload = {
    co2: Number(context.state.co2),
    avgCO2: Number(avgCO2.toFixed(2)),
    motion_per_min: Number(context.state.motion_per_min),
    avgMotion: Number(avgMotion.toFixed(2)),
    quality_level: Number(context.state.quality_level),
    fan_running: context.state.fan_running,
    light_running: context.state.light_running,
    system_enabled: context.state.system_enabled,
    auto_mode: context.state.auto_mode,
    threshold: context.state.threshold,
    fan_cycles: context.state.fan_cycles,
    fan_on_time: context.state.fan_on_time
};

return msg;
