# **Game Cards Server Side – Authoritative LAN Engine**

---

## **Overview**

This project implements an **authoritative server-side engine** for a multiplayer card game operating within a **Local Area Network (LAN)**. The server is the single source of truth, responsible for enforcing rules, managing connections, and synchronizing the game state across all connected clients.

All critical logic is executed on the server to ensure **fair play**, **consistency**, and **security against client-side manipulation**.

---

## **Key Features**

### **Authoritative Server Model**

* Centralized control over all game mechanics
* Server-side deck shuffling, card dealing, and move validation
* Eliminates trust in client-side logic to prevent cheating

### **Multi-threaded Communication**

* Built using **Indy (TIdTCPServer)** for TCP/IP networking
* Handles multiple concurrent client connections
* Non-blocking architecture keeps the VCL UI responsive

### **Real-time Game Synchronization**

* Event-based broadcast system for instant updates
* Supports commands such as:

  * `ATTACK`
  * `YOURTURN`
  * `COMINGDECK`
  * `GAME_FINISHED_DECK_EMPTY`

### **Administrative Monitoring**

* Integrated **VCL-based control panel**
* Live player status monitoring:

  * Ready
  * In-Game
  * My Turn
* Manual session reset for administration and testing

---

## **Project Structure**

The project is divided into three core logical modules:

---

### **1. Backend.cpp – Network & UI Core**

Acts as the central coordination layer between the user interface and the networking subsystem.

**Responsibilities:**

* Player connection management (up to 4 players)
* Slot assignment and clean disconnection handling
* Command processing (`READY`, `DONE`, `MYPOINTS`, `EXIT`)
* Score aggregation and winner determination
* Broadcasting final results to all connected clients

---

### **2. GameRules.cpp – Game Logic & State Machine**

Encapsulates the full rule set and controls the flow of a match.

**Responsibilities:**

* Game state transitions:

  * Waiting
  * Countdown
  * Memorization (10 seconds)
  * Active Gameplay
* Turn-based system with automatic turn rotation
* Sends `YOURTURN` together with a newly drawn card
* Internal Tick-based timer for phase synchronization

---

### **3. Player.cpp – Player & Card Management**

Manages player data, card distribution, and shared resources.

**Responsibilities:**

* Player lifecycle and data management
* Secure 52-card deck generation
* Guaranteed no-duplicate card distribution
* Thread safety using `TCriticalSection`
* Deck exhaustion detection and end-game signaling

---

## **Technical Stack**

* **IDE:** Embarcadero RAD Studio (C++ Builder)
* **Language:** Modern C++ (STL containers, RAII principles)
* **Networking:** Indy TCP/IP components
* **Framework:** VCL (Visual Component Library)
* **Concurrency:** Multi-threaded server model with critical sections

---

## **Operational Workflow**

### **Lobby Phase**

* Server waits for a minimum of **two players**
* Players signal readiness using the `READY` command

### **Dealing Phase**

* Cards are generated exclusively on the server
* Cards are distributed using the `COMINGDECK` protocol

### **Active Gameplay**

* Turn loop controlled entirely by the server
* Server waits for client confirmations:

  * `DONE`
  * `DONEAT`
* Turn automatically passes to the next player

### **Finalization Phase**

* Triggered when:

  * The deck becomes empty, or
  * All players finish their hands
* Server calculates final scores
* Winner is announced to all clients
* Game state is reset for a new round

---

## **Summary**

This server provides a **robust, secure, and extensible foundation** for LAN-based multiplayer card games. Its authoritative architecture, real-time synchronization, and modular design make it suitable for both academic use and fully playable local multiplayer environments.
