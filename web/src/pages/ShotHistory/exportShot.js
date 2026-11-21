export function round2(v) {
  if (v == null || Number.isNaN(v)) return v;
  return Math.round((v + Number.EPSILON) * 100) / 100;
}

// Shapes a shot for JSON export; shared by the per-shot card and the bulk export
export function toExportShot(shot, notes = shot.notes ?? null) {
  const exportData = { ...shot, notes };
  if (Array.isArray(exportData.samples)) {
    exportData.samples = exportData.samples.map(s => ({
      t: s.t,
      tt: round2(s.tt),
      ct: round2(s.ct),
      tp: round2(s.tp),
      cp: round2(s.cp),
      fl: round2(s.fl),
      tf: round2(s.tf),
      pf: round2(s.pf),
      vf: round2(s.vf),
      v: round2(s.v),
      ev: round2(s.ev),
      pr: round2(s.pr),
      wp: round2(s.wp),
      // Undefined, and so omitted, for shots recorded before v9.
      rp: round2(s.rp),
      // v10 test build only; omitted everywhere else.
      po: round2(s.po),
      systemInfo: s.systemInfo,
      phaseNumber: s.phaseNumber,
      phaseDisplayNumber: s.phaseDisplayNumber,
    }));
  }
  exportData.volume = round2(exportData.volume);
  return exportData;
}
