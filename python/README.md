# fprime-cfs

Tooling for running the [cFS GroundSystem](https://github.com/nasa/cFS-GroundSystem) as the
GDS for an F Prime deployment integrated through `fprime_cfs`.

## Installation

```bash
pip install ./python  # from the fprime_cfs checkout root
```

## Tools

### `fprime-cfs`

Runs the full cFS GroundSystem GDS: generates the GroundSystem configuration from the
F Prime dictionary, starts the TM/TC frame bridge to the `fprime_gds` GdsBridge cFS
application, and launches the GroundSystem GUI.

```bash
fprime-cfs --dictionary <deployment>/dict/*TopologyDictionary.json \
    --ground-system-dir <cfs-checkout>/tools/cFS-GroundSystem
```

The GroundSystem GUI has its own dependencies (PyQt5, pyzmq); install them from the
cFS-GroundSystem checkout before running.

### `fprime-cfs-config`

Generates the cFS GroundSystem configuration only:

- `Subsystems/tlmGUI/telemetry-pages.txt` and `Subsystems/tlmGUI/fprime-tlm.txt`: a
  telemetry page for the single fixed telemetry packet defined by the dictionary. Exactly
  one packet must be defined; the tool errors otherwise. Multi-byte items use an explicit
  big-endian (`>`) struct prefix as F Prime serializes values big-endian.
- `Subsystems/cmdGui/command-pages.txt`, `CommandFiles/`, and `ParameterFiles/`: a command
  page exposing every F Prime command. All commands share the single cFS command function
  code published in the dictionary as `ComCfg.FprimeCommandFunctionCode`. The F Prime
  framing descriptor and the command opcode are prepended as the first two parameters of
  every command; their fixed required values are given in each parameter description.
  Commands with arguments that MiniCmdUtil cannot represent (e.g. strings) are skipped
  with a warning.

```bash
fprime-cfs-config --dictionary <dictionary.json> --ground-system <cFS-GroundSystem dir>
```

### `fprime-cfs-comm`

Runs only the communication bridge: TCP CCSDS TM/TC frames (GdsBridge, default port
15010) on one side, bare space packets over UDP (GroundSystem telemetry port 2234,
command port 1234) on the other.

```bash
fprime-cfs-comm --dictionary <dictionary.json>
```
