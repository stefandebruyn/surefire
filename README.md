**NOTE: This project is WIP.** The master branch contains features that are complete and tested:

* [x] State vector, state machine, and task APIs
* [x] Compilers and autocoders for state vector and state machine config languages
* [x] Socket, thread, digital I/O, and analog I/O abstractions
* [ ] Task execution API
* [ ] Configurable network and device I/O tasks
* [ ] Documentation and examples

---

<p align="center">
    <image src="docs/assets/surefire-patch.png" width="200px">
</p>

Surefire is a C++ framework for building flight software applications at the level of advanced collegiate rocketry. The framework provides three main things:

1. Portable interfaces for common flight software needs
2. Config languages to allow rapid iteration of flight software
3. State machine domain-specific language (DSL) to streamline integration of domain-specific control logic

## Getting Started

1. Read the [design one-pager](docs/design-one-pager.md)
2. Work through the [basic application bringup tutorial](docs/basic-application-bringup.md)
3. Find further documentation in the [documentation index](docs/index.md)
4. Full send

## Supported Platforms

Surefire natively supports Linux and sbRIO-9637, with some support for Arduino platforms. The Surefire Core Library is written in bare metal C++11 and built on an abstraction layer, so the user may develop custom ports.

---

## Origin

Surefire was originally developed as a computer science honors thesis at the University of Texas at Austin. The original thesis can be found in the repo [here](docs/debruyn-honors-thesis.pdf). The halcyon bird, delta, and sun cross on the mission patch are borrowed from mission patches of the [Texas Rocket Engineering Lab](https://www.texasrocketlab.com), whose flight software was a spiritual predecessor to Surefire.
