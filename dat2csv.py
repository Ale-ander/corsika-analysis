#!/usr/bin/env python3
import sys
import numpy as np
from corsikaio import CorsikaParticleFile
from particle import Corsika7ID

def to_pdg(cid: int) -> int:
    try:
        return int(Corsika7ID(cid).to_pdgid())
    except Exception:  # ID non mappabile (record speciali, muoni 75/76, ecc.)
        return 0

def main(datfile, out="shower"):
    with CorsikaParticleFile(datfile) as f, open(out + ".csv", "w") as csv:
        csv.write("event,pdg,x_cm,y_cm,t_ns,px_GeV,py_GeV,pz_GeV\n")

        for ev_idx, event in enumerate(f, start=1):
            p = event.particles
            corsika_id = (p["particle_description"] // 1000).astype(int)
            pdg_ids = [to_pdg(int(c)) for c in corsika_id]

            rows = []
            for k in range(len(pdg_ids)):
                c = pdg_ids[k]
                x, y, t = p["x"][k], p["y"][k], p["t"][k]
                px, py, pz = p["px"][k], p["py"][k], -p["pz"][k]
                csv.write(f"{ev_idx},{c},{x:.4f},{y:.4f},{t:.4f},{px:.8e},{py:.8e},{pz:.8e}\n")

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else "shower")