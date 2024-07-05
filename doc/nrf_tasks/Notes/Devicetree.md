Hardware is traditionally described inside header files (`.h` or `.hh`). nRF Connect SDK uses a method to describe hardware borrowed from the Zephyr RTOS, which is through a construct called a devicetree.
The devicetree uses a specific format consisting of _nodes_ connected together, where each node contains a set of _properties_.

Here is an example DTS(**d**evice**t**ree **s**ource) file: with 3 nodes /, a-node & a-sub-node.

```c-like
/dts-v1/;
/ {
        a-node {
                subnode_label: a-sub-node {
                        foo = <3>;
                };
        };
};
```

- label: ``subnode_label`` which can be used elsewhere in dts file.
- properties: ``foo`` is property it can have any value strings, bytes, numbers, or any mixture of types
### Devicetree bindings (YAML files)
- Every devicetree node must have a compatible property.
- Below is an example of a devicetree binding file (`.yaml`) that defines the compatible property named `nordic,nrf-sample` with one required property named `num-sample` of type integer.

```c-like
compatible: "nordic,nrf-sample"
properties:
  num-sample:
    type: int
    required: true
```
- Below is a sample DTS file (`.dts`) with the node `node0` that is set to the compatible `nordic,nrf-sample`. This means the `node0` node must have the required property `num-sample` and that property must be assigned an integer value. Otherwise, the build will fail.

```c-like
node0 {
     compatible = "nordic,nrf-sample";
     num-sample = <3>;
};
```


#### Referances
- [devicetree bindings](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/dts/api/bindings.html#dt-vendor-nordic)