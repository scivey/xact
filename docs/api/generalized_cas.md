## The Generalized CAS API

This API is a generalization of compare-and-swap which allows for multiple predicate-based preconditions, as well as multiple effects when those preconditions are met.

### What?

To clarify, think of the `cmpxchg` instruction as a combination of one precondition and one effect:
```
[precondition] The value at address A is expected to equal X

[if preconditions satisfied....]
[effect]       Store Y at address A.
```

The Generalized CAS API is similar, except that it allows multiple preconditions and effects to be combined.  This means that the following can be performed as one atomic operation:
```
[precondition] The value at address A is expected to equal 10
[precondition] The value at address B is expected to be greater than 100
[precondition] The value at address B is expected to be less than 200
[precondition] The value at address C is expected not to equal 17
[precondition] The value at address D is expected to be greater than 1000
[precondition] The value at adresss E is greater than 0

[if preconditions satisfied...]
[effect]       Store the value 20 at address A
[effect]       Atomically increment the value at B by 5
[effect]       Store the value 17 at address C
[effect]       Atomically decrement the value at D by 1
```

This interface does have limitations: it doesn't allow for more general logic or branching, and it can't currently model dependencies between target memory locations well.  Despite this, it's still significantly more expressive than a single compare-and-swap.

### The API and Examples

This API was developed pretty recently - better docs will be up shortly.
Currently, the [unit tests](/src/test/unit/generalized_cas_1/test_GeneralizedCAS.cpp) are the best documentation.

