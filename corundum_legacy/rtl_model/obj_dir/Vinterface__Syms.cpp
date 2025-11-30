// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table implementation internals

#include "Vinterface__Syms.h"
#include "Vinterface.h"

// FUNCTIONS
Vinterface__Syms::Vinterface__Syms(Vinterface* topp, const char* namep)
	// Setup locals
	: __Vm_namep(namep)
	, __Vm_didInit(false)
	// Setup submodule names
{
    // Pointer to top level
    TOPp = topp;
    // Setup each module's pointers to their submodules
    // Setup each module's pointer back to symbol table (for public functions)
    TOPp->__Vconfigure(this, true);
    // Setup scope names
    __Vscope_interface__axil_interconnect_inst.configure(this,name(),"interface.axil_interconnect_inst");
    __Vscope_interface__port__BRA__0__KET____port_inst__axil_interconnect_inst.configure(this,name(),"interface.port[0].port_inst.axil_interconnect_inst");
}
