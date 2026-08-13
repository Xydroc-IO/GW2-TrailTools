# GW2-TrailTools

**v1.0.0** — TacO/Blish trail & marker pack authoring for [Raidcore Nexus](https://raidcore.gg/gw2/nexus).

Documentation lives under **[`docs/`](docs/)**:

| Doc | |
|-----|--|
| [README](docs/README.md) | Overview, build, install, branding |
| [RELEASE_NOTES](docs/RELEASE_NOTES.md) | 1.0.0 player-facing notes |
| [CHANGELOG](docs/CHANGELOG.md) | Short version history |
| [ARCHITECTURE](docs/ARCHITECTURE.md) | Domains & runtime layout |
| [CONTRIBUTING](docs/CONTRIBUTING.md) | Dev rules (≤500 lines / `.cpp`) |
| [COMPLIANCE](docs/COMPLIANCE.md) | Allowed / denied APIs |
| [SECURITY](docs/SECURITY.md) | Reporting |

```bash
make -j"$(nproc)" && make check-lines
```
