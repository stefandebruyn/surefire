# Design One-Pager

## Design Goals

In no particular order:

1. **Simplicity** - Limit the size of interfaces, memory footprint, and third-party dependencies
2. **Portability** - Enable deployment on any platform, from microcontrollers to highly provisioned embedded boxes
3. **Flexibility** - Support a variety of applications and use cases, especially those which iterate rapidly
4. **Fault Tolerance** - Minimize opportunities for error and surface all errors
5. **Layman Programmability** - Enable non-software engineers to program control logic

## Framework Architecture

Surefire is divided into three layers. From highest level to lowest:

1. **Core Library** - Main API layer used to build flight software applications
2. **Platform Abstraction Layer (PAL)** - Abstract interfaces to platform APIs like sockets, threads, etc.
3. **Platform Support Layer (PSL)** - Platform-specific implementations of PAL interfaces

Ideally, a Surefire application accesses only the Core Library and possibly PAL. Surefire can be ported to a new platform by creating a new PSL. PSL components can be mixed and matched, e.g., by reusing parts of the native Linux PSL to bring up a new Linux platform.

## Application Architecture

A Surefire application performs real-time control via an **executor**, or scheduler. The executor periodically executes **tasks** according to some schedule. A task can be anything; common examples are control algorithms and device drivers. Tasks communicate through an area of shared memory called the **state vector**. The state vector is divided into scalar data **elements**. Tasks on different machines communicate by sharing elements of their state vectors over the network. Tasks may be implemented by **state machines** that operate on state vector elements.

## Application Configuration

The main Core Library APIs are configurable from text files, which are processed by a separate library called the Config Library. The Config Library can be linked to the user application in order to load config files at runtime. If the target platform cannot support the Config Library (which uses the C++ Standard Library and heap), then the Config Library can be used to generate C++ autocode from config files. The autocode is then compiled into the user application as static configuration data. Config files target areas of flight software that are likely to change often over the development of an application, such as the state vector layout and state machines.

## State Machine DSL

 State machine config files are written in a statically-typed, imperative domain-specific language (DSL) designed for use by domain experts that are not necessarily programmers. The goal is to streamline integration of domain-specific control logic (like engine ignition sequences or fluids conops) by allowing domain experts to express the logic in an intermediate form that can be automatically translated into flight software. If the target platform supports the Config Library, this can allow live state machine updates by recompiling a DSL file on-target in the middle of runtime. This has applications for vehicle operations where the control software cannot be safely taken offline for maintenance mid-operation, such as tests involving live propellants.
