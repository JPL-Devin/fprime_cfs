# ======================================================================
# CfsCmdRouterCfg.fpp (unit test configuration override)
# Routing table exercising all route categories for the CfsCmdRouter unit tests
# ======================================================================
module FPrimeCfs {

    @ Number of com output ports on the CfsCmdRouter
    constant CFS_CMD_ROUTER_COM_PORTS = 10

    @ Number of buffer output ports on the CfsCmdRouter
    constant CFS_CMD_ROUTER_BUFFER_PORTS = 10

    @ Maximum number of buffers simultaneously outstanding on the pass-through routes
    constant CFS_CMD_ROUTER_MAX_PENDING_BUFFERS = 10

    @ Number of entries in the function code routing table
    constant CFS_CMD_ROUTER_ROUTE_TABLE_SIZE = 2

    @ Test routing table with one route of each category
    constant CFS_CMD_ROUTER_ROUTE_TABLE = [
        { functionCode = 0, routeType = CfsCmdRouteType.COM,    portIndex = 0 },
        { functionCode = 1, routeType = CfsCmdRouteType.BUFFER, portIndex = 0 },
    ]
}
