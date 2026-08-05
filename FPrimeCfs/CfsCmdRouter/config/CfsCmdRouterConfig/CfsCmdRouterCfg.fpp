# ======================================================================
# CfsCmdRouterCfg.fpp
# Constants for configuring the CfsCmdRouter routing table and port counts
#
# Projects override this file (register_fprime_config CONFIGURATION_OVERRIDES)
# to supply their own function code routing table. Function codes not present
# in the table take the unknown route.
# ======================================================================
module FPrimeCfs {

    @ Number of com output ports on the CfsCmdRouter
    constant CFS_CMD_ROUTER_COM_PORTS = 10

    @ Number of buffer output ports on the CfsCmdRouter
    constant CFS_CMD_ROUTER_BUFFER_PORTS = 10

    @ Maximum number of buffers simultaneously outstanding on the pass-through
    @ routes (buffer, unknown) awaiting return
    constant CFS_CMD_ROUTER_MAX_PENDING_BUFFERS = 10

    @ Number of entries in the function code routing table
    constant CFS_CMD_ROUTER_ROUTE_TABLE_SIZE = 2

    @ The function code routing table: function code -> (route type, output port index).
    @ This default configuration routes function code 0 (ComCfg.FprimeCommandFunctionCode,
    @ the CfsBridge F Prime command wrapping function code) as a bare com; projects
    @ override with their own codes. Projects overriding ComCfg.FprimeCommandFunctionCode
    @ must override this table to match: the literal here cannot reference the ComCfg
    @ constant as ComCfg is not visible to this configuration module.
    constant CFS_CMD_ROUTER_ROUTE_TABLE = [
        { functionCode = 0, routeType = CfsCmdRouteType.COM,    portIndex = 0 },
        { functionCode = 1, routeType = CfsCmdRouteType.BUFFER, portIndex = 0 },
    ]
}
