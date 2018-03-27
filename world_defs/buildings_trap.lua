-- Trap buildings

buildings["cage_trap"] = {
    name = "Cage Trap",
    description = "A wicked trap designed to catch invaders inside a cage.",
    components = { { item="cage", qty=1}, { item="mechanism", qty=1 } },
    skill = { name="Construction", difficulty=10 },
    provides = { cage_trap={energy_cost=0} },
    render_rex = "cagetrap.xp",
    vox = voxelId("cage")
};

buildings["stonefall_trap"] = {
    name = "Stonefall Trap",
    description = "A simple trap that drops a rock on an invader's head.",
    components = { { item="stone_boulder", qty=1}, { item="mechanism", qty=1 } },
    skill = { name="Construction", difficulty=10 },
    provides = { stonefall_trap={energy_cost=0} },
    render_rex = "cagetrap.xp",
    vox = voxelId("stonefall")
};

buildings["weapon_trap"] = {
    name = "Blade Trap",
    description = "A nasty trap designed to slice and dice invaders.",
    components = { { item="trap_blade", qty=1}, { item="mechanism", qty=1 } },
    skill = { name="Construction", difficulty=10 },
    provides = { blade_trap={energy_cost=0} },
    render_rex = "cagetrap.xp",
    vox = voxelId("blades_retracted")
};

buildings["spike_trap"] = {
    name = "Retractable Spikes",
    description = "Sharp spikes that can be retracted into the ground, or extended to damage all present.",
    components = { { item="retractable_spike", qty=1}, { item="mechanism", qty=1 } },
    skill = { name="Construction", difficulty=12 },
    provides = { spike_trap={energy_cost=0} },
    render_rex = "cagetrap.xp",
    vox = voxelId("spikes_retracted")
};

buildings["lever"] = {
    name = "Lever",
    description = "The most basic input form for a machine, levers can be linked to other devices to operate them.",
    components = { { item="mechanism", qty=1 } },
    skill = { name="Construction", difficulty=12 },
    provides = { lever={energy_cost=0} },
    render = {
        width=1, height=1, tiles={
            {glyph=glyphs['lever_off'], foreground = colors['red'], background = colors['black']}
        }
    },
    render_ascii = {
        width=1, height=1, tiles={
            {glyph=glyphs['marker'], foreground = colors['red'], background = colors['black']}
        }
    },
    vox = voxelId("lever_off")
};

buildings["pressure_plate"] = {
    name = "Pressure Plate",
    description = "A simple pressure plate that triggers when something stands on it.",
    components = { { item="mechanism", qty=1 } },
    skill = { name="Construction", difficulty=12 },
    provides = { pressure_plate={energy_cost=0} },
    render_rex = "pressureplate.xp",
    vox = voxelId("pressure_plate")
};

buildings["float_sensor"] = {
    name = "Float sensor",
    description = "A simple water level gauge, triggering if the water rises above a certain level.",
    components = { { item="mechanism", qty=1 }, { item="block", qty=1 } },
    skill = { name="Construction", difficulty=12 },
    provides = { float_sensor={energy_cost=0} },
    render_rex = "pressureplate.xp",
    vox = voxelId("float_gauge")
};

buildings["proximity_sensor"] = {
    name = "Proximity sensor",
    description = "An electronic proximity sensor that triggers if movement occurs nearby.",
    components = { { item="mechanism", qty=1 }, { item="circuit", qty=1 } },
    skill = { name="Construction", difficulty=12 },
    provides = { float_sensor={energy_cost=0} },
    render_rex = "pressureplate.xp",
    vox = voxelId("proximity_sensor")
};
