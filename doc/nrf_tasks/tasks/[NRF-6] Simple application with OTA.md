---
Epic: "[NRF-E3] Multi core deployment and OTA"
Created: 2024-07-03
Due: 
Status: Backlog
Assigned to: 
tags:
  - BLE_UART
  - Node
  - "#OTA"
Git:
---
#### Description
nRF supports OTA and there is separate process to build and deploy the OTA updates.
Things to learn:
- MCUBoot (introduction)
- OTA build and update
#### Sub-Task
- [ ] Use node application used in [[[NRF-5] 1C - 2P Communication bridge|NRF-5]] and make it OTA compliant
- [ ] Update application to comply with following sequence
```mermaid
sequenceDiagram
    Node1->>+Central: send 'start(counter)' command
    Central->>+Node2: 
    Node2->>Central: [(RAD1+5)..(RAD5+5)]
    Central->>+Node1: 
    Node1->>Node1: calculate SHA-1/SHA-2 hash
    Node1-->>-Central: Hash of 5 numbers
    Central-->>Node2: Hash of 5 numbers
    Node2->>Node2: Validate hash
    alt same
        Node2-->>Central: completed
        Central-->>+Node1: 
        Node1->>-Node1: Log completion status and update counter
    else different
        Node2-->>-Central: Mismatch
        Central-->>-Node1: 
    end
```

#### Task Relationships
- Depends on:
	- [[[NRF-5] 1C - 2P Communication bridge]]