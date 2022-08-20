**NOTE: This project is in beta. The master branch is tested and functional, but work is ongoing.**

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

1. Read the [design one-pager](./design-one-pager.md)
2. Work through the [basic application bringup tutorial]()
3. Find further documentation in the [documentation index]()
4. Full send

## Supported Platforms

Surefire natively supports Linux and NILRT, with some support for Arduino platforms. The Surefire Core Library is written in bare metal C++11 and built on an abstraction layer, so the user may develop custom ports.

## Requirements

* Core Library - C++11 (does not require a standard library or heap)
* Config Library - C++11, C++ Standard Library, heap, GNU 4.9.2+

## History

Surefire was originally developed as a computer science honors thesis at the University of Texas at Austin. The original thesis can be found in the repo [here](docs/debruyn_honors_thesis.pdf).
