# Scenegraph Instancing {#Usd_Page_ScenegraphInstancing}

## Overview {#Usd_ScenegraphInstancing_Overview}

USD's instancing functionality allows prims that bring in common scene 
description via <a href="https://openusd.org/release/glossary.html#usdglossary-compositionarcs">composition arcs</a> 
to share those parts of the scenegraph, rather than having them duplicated for 
each prim.  This can greatly improve performance in cases where many copies of 
the same asset are brought in to a scene via composition.  For example, a 
parking lot scene may use references to bring in the same car model (or set of 
car models) hundreds or thousands of times to fully populate the lot.  In cases
like these, instancing can provide significant performance benefits.

\anchor image_overview_scenegraph
\image html instancing/Uninstanced_vs_Instanced.png "Uninstanced vs. Instanced Scenegraph"

With instancing, the hierarchy of prims from the referenced Car model is
exposed beneath a single prototype prim, instead of having _n_ copies of that
hierarchy beneath each of the Car prims in the scene.  This reduces the size
of the scenegraph, which reduces the time needed to load a UsdStage
as well as the memory it consumes.  

Consumers can take advantage of the shared scenegraph to reduce the amount of 
processing they need to do.  For example, a consumer that needs to compute the 
bounding box for all of the Car prims in the scene could compute the bounding 
box for the shared scenegraph just once and reuse that result instead of 
redoing the same computation for each one.  To allow the scenegraph to be
shared in this way, instancing restricts the ability to override prims and
properties in the prototype prim on a per-instance basis.  In this example,
consumers would not be able to add, remove, or override prims beneath any
of the Car prims in the scene.  With this restriction, instancing is able to
provide higher degrees of scalability; the benefits become more and more 
significant as the number of Car prims increases, or as the number of prims 
beneath the referenced Car model grows.

## Explicit Instances, Implicit Prototypes {#Usd_ScenegraphInstancing_Instancing}

Prims that share parts of the scenegraph through instancing are called
"instance" prims.  Each of these instance prims are associated with a
"prototype" prim that serves as the root of the shared scenegraph.  A single
prototype prim may have many associated instances, all of which share the same
scenegraph.  

Users do not explicitly set up the network of prototype and instance prims.
Instead, users mark the prims they want to be instanced with metadata that 
tags them as "instanceable."  UsdStage will analyze these prims to determine 
which have scenegraph in common based on their composition structure, then 
dynamically create prototype prims for each group of compatible instances. 
This "explicit instance, implicit prototype" scheme provides a simple and 
flexible way for adding instanced assets to scenes.  See \ref Usd_ScenegraphInstancing_Instanceable
for information on how to tag prims for instancing.

<center>
<table border="0" style="table-layout: fixed;" width="75%">
<caption>Uninstanced vs. Instanced Scene Description</caption>
<tr>
<td valign="top">
\code
### ParkingLot.usd

#usda 1.0

def "ParkingLot"
{
    def "Car_1" (references = @./Car.usd@</Car>)
    {
    }

    def "Car_2" (references = @./Car.usd@</Car>)
    {
    }

    # ...

    def "Car_n" (references = @./Car.usd@</Van>)
    {
    }
}
\endcode
</td>
<td valign="top">
\code
### ParkingLot.usd

#usda 1.0

def "ParkingLot"
{
    def "Car_1" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }

    def "Car_2" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }

    # ...

    def "Car_n" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }
}
\endcode
</td>
</tr>
</table>
</center>

This is a simple example of what the scene description in a .usd file might 
look like for the scenegraph above.  In this case, UsdStage will recognize that 
all of the instanceable Car prims reference the same Car model and will have 
the same scenegraph hierarchy.  UsdStage will then generate a prototype prim
/__Prototype_1 to contain this common hierarchy and associate the Car instances 
with this prototype.

Prototype prims do not exist in scene description -- they are generated and 
managed internally by UsdStage.  This allows UsdStage to create and remove 
prototypes as needed in response to scene description changes.  For example, 
if some of the Car prims in ParkingLot.usd were changed to reference different 
assets, UsdStage would generate new prototype prims as needed.
See \ref Usd_ScenegraphInstancing_Prototypes for information on how to use
prototype prims.

\warning Because prototype prims are dynamically generated, the name of a
prototype prim associated with an instance is not stable and may vary from
run-to-run. Consumers should not save or hard-code the paths to prims in
prototypes, but can use the API described below if they need to determine an
instance's prototype at runtime.

## Working with Instancing {#Usd_ScenegraphInstancing_Querying}

This section goes into more detail about instance and prototype prims and the
API for working with them.  The following example will be used throughout:

<center>
<table border="0">
<caption>Example Scene Description and Scenegraph</caption>
<tr>
<td valign="top">
\code
### ParkingLot.usd

#usda 1.0

def "ParkingLot"
{
    def "Car_1" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
        color3f color = (1, 0, 0)
    }

    def "Car_2" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
        color3f color = (0, 1, 0)
    }

    def "Car_3" (
        references = @./Car.usd@</Car>
    )
    {
        color3f color = (0, 0, 1)
    }
}
\endcode

\code
### Car.usd

#usda 1.0

def "Car"
{
    color3f color = (0, 0, 0)

    def Mesh "Body"
    {
        color3f color = (0, 0, 0)
    }

    def Mesh "Door"
    {
    }
}
\endcode
</td>
<td valign="middle">
\image html instancing/Instancing_Example.png 
</td>
</tr>
</table>
</center>

### Making Prims Instanceable {#Usd_ScenegraphInstancing_Instanceable}

UsdStage can only create prototype prims for portions of the scenegraph
that are brought into a scene via a composition arc.  Because of this,
a prim must use at least one composition arc in order to be eligible 
for instancing.

Prims may be marked as instanceable by using UsdPrim::SetInstanceable or
SdfPrimSpec::SetInstanceable.  If no value is explicitly authored, this value 
defaults to \c false.

The "instanceable" metadata is composed like all other metadata in USD, so it 
can be authored on different layers and have its value overridden by stronger 
layers.  For example, a user could author their "instanceable" metadata in a 
session layer to see the effects of enabling (or disabling) instancing on prims 
in a scene without editing that scene directly.

### Classifying Prims with Instancing {#Usd_ScenegraphInstancing_Instances}

Consumers can check if a UsdPrim is an instance with an associated prototype
using the following API.

| Function | Purpose | Example |
| -------- | ------- | ------- |
| UsdPrim::IsInstance | Check if prim is an instance. | Returns true for Car_1 and Car_2 since they are instances, but returns false for Car_3. |

### Finding and Traversing Prototypes {#Usd_ScenegraphInstancing_Prototypes}

A prototype prim is a special UsdPrim whose sole purpose is to serve as the 
parent for the scenegraph shared by its associated instance prims.  The
following API can be used to retrieve prototype prims or determine whether a prim
is part of a shared prototype.

| Function | Purpose | Example |
| -------- | ------- | ------- |
| UsdStage::GetPrototypes | Return a list of all prototype prims on the stage | Returns the prim /__Prototype_1 |
| UsdPrim::GetPrototype | If prim is an instance, get the corresponding prototype prim. | Returns prim /__Prototype_1 for Car_1 or Car_2, an invalid UsdPrim for all other prims, including Car_3. |
| UsdPrim::IsPrototype  | Check if this prim is a prototype prim. | Returns true for prim /__Prototype_1, false for all other prims. |
| UsdPrim::IsInPrototype | Check if this prim is in a subtree rooted at a prototype prim. | Returns true for prims /__Prototype_1, /__Prototype_1/Body, and /__Prototype_1/Door, false for all other prims. |

Prototype prims do not have metadata or properties, only children prims.
Prototype prims have root prim paths (e.g., /__Prototype_1) that can be used
with UsdStage::GetPrimAtPath.  However, they are considered siblings to the 
pseudo-root in the scenegraph.  Because of this, they will not be returned when 
using UsdStage::Traverse or when calling UsdPrim::GetChildren on the 
pseudo-root.  Consumers can access prototype prims via UsdPrim::GetPrototype or 
UsdStage::GetPrototypes instead.

\warning Because prototype prims are dynamically generated, the name of a
prototype prim associated with an instance is not stable and may vary from
run to run. Consumers should not save or hard-code the paths to prims in
prototypes, but can use the API described below if they need to determine an
instance's prototype at runtime.

\code{.py}
>>> stage = Usd.Stage.Open('ParkingLot.usd')
>>> stage.GetPrototypes()
[Usd.Prim(</__Prototype_1>)]

# Prototype prims are not included in stage traversals.  Note how the below
# functions do not include the prototype prims above.
>>> list(stage.Traverse())
[Usd.Prim(</ParkingLot>), Usd.Prim(</ParkingLot/Car_1>), Usd.Prim(</ParkingLot/Car_2>), Usd.Prim(</ParkingLot/Car_3>), Usd.Prim(</ParkingLot/Car_3/Body>), Usd.Prim(</ParkingLot/Car_3/Door>)]

>>> stage.GetPseudoRoot().GetChildren()
[Usd.Prim(</ParkingLot>)]

\endcode

UsdStage arranges the scenegraph so that instance prims do not have any 
descendant prims; these prims are instead encapsulated beneath prototype prims.
Consumers can use UsdPrim::GetPrototype to get an instance's prototype prim,
then traverse its children the same way as with other prims, using 
UsdPrim::GetChildren, UsdPrimRange or similar facilities. 

\code{.py}
>>> stage = Usd.Stage.Open('ParkingLot.usd')

# Car_1 doesn't make any child prims available since it's an instance prim;
# its children in the scenegraph are parented beneath the prototype prim.
>>> car_1 = stage.GetPrimAtPath('/ParkingLot/Car_1')
>>> car_1.IsInstance()
True
>>> car_1.GetChildren()
[]

# Consumers can query the instance's prototype for its child prims.
>>> car_1.GetPrototype().GetChildren()
[Usd.Prim(</__Prototype_1/Body>), Usd.Prim(</__Prototype_1/Door>)]
>>> list(Usd.PrimRange(car_1.GetPrototype()))
[Usd.Prim(</__Prototype_1>), Usd.Prim(</__Prototype_1/Body>), Usd.Prim(</__Prototype_1/Door>)]

# Car_3's child prims can be accessed directly since it's not an instance prim.
>>> car_3 = stage.GetPrimAtPath('/ParkingLot/Car_3')
>>> car_3.IsInstance()
False
>>> car_3.GetChildren()
[Usd.Prim(</ParkingLot/Car_3/Body>), Usd.Prim(</ParkingLot/Car_3/Door>)]

\endcode

### Traversing Into Instances with Instance Proxies {#Usd_ScenegraphInstancing_InstanceProxies}

An instance proxy is a UsdPrim that represents a descendant prim beneath an 
instance, even though no such prim actually exists in the scenegraph.  Instance
proxies allow consumers to work with the scenegraph as if instancing were not 
being used, while retaining the load time and memory usage benefits of
instancing.  

An instance proxy prim behaves the same as the corresponding prim in the 
prototype.  However, an instance proxy retains context about the instance being 
traversed so they appear to be a prim beneath an instance: its path is a 
descendant of the instance being traversed instead of a descendant of a
prototype prim.  Consumers should generally be able to use instance proxies
anywhere a UsdPrim is used.  The primary exception is that editing scene
description via instance proxies and their properties is not allowed.

Calling UsdStage::GetPrimAtPath with a path to a descendant of an instance
prim will return an instance proxy if a corresponding prim exists in that
instance's prototype.  By default, the various prim traversal facilities like 
UsdPrim::GetChildren and UsdPrimRange do not return instance proxies.  
#UsdTraverseInstanceProxies can be used to enable this functionality.  By
default, this uses the same #UsdPrimDefaultPredicate used by other traversal
functions,  but it can be combined with other \ref Usd_PrimFlags predicates; 
for example, combining it with #UsdPrimAllPrimsPredicate would yield the same 
filtering behavior as UsdPrim::GetAllChildren, but with instance proxies 
enabled.

\code{.py}
>>> stage = Usd.Stage.Open('ParkingLot.usd')

# Car_1 doesn't make any child prims available by default since it's an
# instance prim.
>>> car_1 = stage.GetPrimAtPath('/ParkingLot/Car_1')
>>> car_1.IsInstance()
True
>>> car_1.GetChildren()
[]

# Use Usd.TraverseInstanceProxies to enable instance proxies with the same 
# filtering that UsdPrim.GetChildren would do.
>>> car_1.GetFilteredChildren(Usd.TraverseInstanceProxies())
[Usd.Prim(</ParkingLot/Car_1/Body>), Usd.Prim(</ParkingLot/Car_1/Door>)]

# Calling Usd.Stage.GetPrimAtPath with the path of a prim beneath an
# instance will also return an instance proxy.
>>> car_1_body = stage.GetPrimAtPath('/ParkingLot/Car_1/Body')
>>> car_1_body
Usd.Prim(</ParkingLot/Car_1/Body>)
>>> car_1_body.IsInstanceProxy()
True

# Unlike prims in prototypes, you can walk up an instance proxy's parent prims
# to find its owning instance.
>>> car_1_body.GetParent()
Usd.Prim(</ParkingLot/Car_1>)

# From an instance proxy, you can retrieve the corresponding prim in the
# instance's prototype.
>>> car_1_body.GetPrimInPrototype()
Usd.Prim(</__Prototype_1/Body>)

# Instance proxies can be used for read-only operations anywhere a Usd.Prim
# is used.
>>> img = UsdGeom.Imageable(car_1_body)
>>> img.ComputeLocalToWorldTransform(Usd.TimeCode.Default())
Gf.Matrix4d(...)

>>> xfCache = UsdGeom.XformCache()
>>> xfCache.GetLocalToWorldTransform(car_1_body)
Gf.Matrix4d(...)

\endcode

### Editing Instances and Prototypes {#Usd_ScenegraphInstancing_Editing}

Properties and metadata (e.g., variant selections) on instance prims can be 
edited and overridden like any other prim.  However, properties and metadata 
on descendant prims beneath instance prims cannot be overridden.  Since
prototype prims are dynamically generated and do not exist in scene description,
overriding properties and metadata on prototypes or prims in prototypes is also
not allowed.  Attempting to make these restricted changes via the USD API will 
result in coding errors.  Any existing overrides in scene description will be 
silently ignored.

\code{.py}
>>> stage = Usd.Stage.Open('ParkingLot.usd')

# Properties on an instance can be overridden as expected.
>>> car1Color = stage.GetPrimAtPath('/ParkingLot/Car_1').GetAttribute('color')
>>> car1Color.Get()
Gf.Vec3f(1.0, 0.0, 0.0)
>>> car1Color.Set((1.0, 1.0, 1.0))
True
>>> car1Color.Get()
Gf.Vec3f(1.0, 1.0, 1.0)

# Properties on prims in prototypes cannot be overridden. This is also the case
# when accessing the property via an instance proxy.
>>> prototype = stage.GetPrimAtPath('/ParkingLot/Car_1').GetPrototype()
>>> prototypeBodyColor = prototype.GetChild('Body').GetAttribute('color')
>>> prototypeBodyColor.Get()
Gf.Vec3f(0.0, 0.0, 0.0)
>>> prototypeBodyColor.Set((1.0, 1.0, 1.0))
pixar.Tf.ErrorException

>>> instanceProxyBody = stage.GetPrimAtPath('/ParkingLot/Car_1/Body')
>>> instanceProxyBodyColor = instanceProxyBody.GetAttribute('color')
>>> instanceProxyBodyColor.Get()
Gf.Vec3f(0.0, 0.0, 0.0)
>>> instanceProxyBodyColor.Set((1.0, 1.0, 1.0))
pixar.Tf.ErrorException

\endcode

These restrictions ensure that the scenegraph in a prototype prim can be shared
by all instances.  If an instance-specific edit to a prim in a prototype is
needed, that instance must be made un-instanceable (see \ref Usd_ScenegraphInstancing_Instanceable)
so that it will no longer participate in instancing.

Although "editing prototypes" is disallowed, USD's composition arcs can be used
to achieve many of the same effects.  For example, a consumer could add inherit
or specializes arcs to instances, then make edits to the class targeted by those
arcs.  Those edits would then affect all of the specified instances.

### Relationship Targets and Attribute Connections {#Usd_ScenegraphInstancing_TargetsAndConnections}

Relationships and attributes may have authored target and connection paths that
point to objects beneath an instance prim.  In these cases, the API for 
retrieving these paths, such as UsdRelationship::GetTargets or 
UsdAttribute::GetConnections, will not translate them to the corresponding 
object in the instance's prototype prim. Consumers can use 
\ref Usd_ScenegraphInstancing_InstanceProxies "instance proxies" to interact 
with these paths and retrieve the corresponding object in the prototype prim if 
needed.  However, if these functions are called on a UsdRelationship or 
UsdAttribute that belong to a prim in a prototype, paths that point to objects
in instances of that prototype will be returned as paths in that prototype.

\note The following example demonstrates behavior with relationship targets,
but the same behavior holds for attribute connections.

<center>
<table border="0">
<caption>Example Scene Description and Scenegraph with Relationships</caption>
<tr>
<td valign="top">
\code
### ParkingLot.usd

#usda 1.0

def "ParkingLot"
{
    def "Car_1" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }

    def "Car_2" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }

    def "ShoppingCart"
    {
        custom rel bodyRel = [
            </ParkingLot/Car_1/Body>,
            </ParkingLot/Car_2/Body>,
        ]
    }
}
\endcode

\code
### Car.usd

#usda 1.0

def "Car"
{
    def Mesh "Body"
    {
        custom rel doorRel = </Car/Door>
    }

    def Mesh "Door"
    {
    }
}
\endcode
</td>
<td valign="middle">
\image html instancing/Relationship_Example.png 
</td>
</tr>
</table>
</center>

\code{.py}
>>> stage = Usd.Stage.Open('ParkingLot.usd')

>>> cart = stage.GetPrimAtPath('/ParkingLot/ShoppingCart')
>>> cart_bodyRel = cart.GetRelationship('bodyRel')

# 'bodyRel' is a relationship on a prim that is not being instanced. Its 
# targets point to prims beneath Car_1 and Car_2, which are both instances 
# in this scene.
>>> cart_bodyRel.GetTargets()
[Sdf.Path('/ParkingLot/Car_1/Body'), Sdf.Path('/ParkingLot/Car_2/Body')]

# Calling Usd.Stage.GetPrimAtPath with these targets will return instance
# proxies, so consumers can easily interact with these targets even though
# they are prims beneath instances.
>>> [stage.GetPrimAtPath(p) for p in cart_bodyRel.GetTargets()]
[Usd.Prim(</ParkingLot/Car_1/Body>), Usd.Prim(</ParkingLot/Car_2/Body>)]

# 'doorRel' is a relationship on a prim beneath an instance. If accessed
# through the instance's prototype, the target paths returned will be returned
# as paths in the instance's prototype.
>>> prototype = stage.GetPrimAtPath('/ParkingLot/Car_1').GetPrototype()
>>> prototype_doorRel = prototype.GetChild('Body').GetRelationship('doorRel')

>>> prototype_doorRel.GetPath()
Sdf.Path('/__Prototype_1/Body.doorRel')

>>> prototype_doorRel.GetTargets()
[Sdf.Path('/__Prototype_1/Door')]

# If the relationship is accessed through instance proxies, the relationship
# target API behaves as though instancing is not present.
>>> instanceProxy = stage.GetPrimAtPath('/ParkingLot/Car_1/Body')
>>> instanceProxy_doorRel = instanceProxy.GetRelationship('doorRel')

>>> instanceProxy_doorRel.GetPath()
Sdf.Path('/ParkingLot/Car_1/Body.doorRel')

>>> instanceProxy_doorRel.GetTargets()
[Sdf.Path('/ParkingLot/Car_1/Door')]

>>> instanceProxy_2 = stage.GetPrimAtPath('/ParkingLot/Car_2/Body')
>>> instanceProxy_2_doorRel = instanceProxy_2.GetRelationship('doorRel')

>>> instanceProxy_2_doorRel.GetPath()
Sdf.Path('/ParkingLot/Car_2/Body.doorRel')

>>> instanceProxy_2_doorRel.GetTargets()
[Sdf.Path('/ParkingLot/Car_2/Door')]
\endcode

## Common Issues {#Usd_ScenegraphInstancing_Issues}

### Instancing Single Prims {#Usd_ScenegraphInstancing_IssuesSinglePrim}

Since instancing shares the scenegraph hierarchy beneath instance prims,
instancing a single prim that has no descendants provides no benefits.  In the
case that instancing a single prim is desired (e.g., instancing a mesh), that
prim should be made a descendant of another prim that is referenced into the
scene, as shown below:

\code
### Model.usd

#usda 1.0

over "InstancedMeshSource"
{
    def Mesh "Mesh"
    {
        # ...
    }
}

def "Model"
{
    def "Mesh_1" (
        instanceable = true
        references = </InstancedMesh>
    )
    {
    }

    def "Mesh_2" (
        instanceable = true
        references = </InstancedMesh>
    )
    {
    }

    # ...
}
\endcode

## Advanced Topics {#Usd_ScenegraphInstancing_Advanced}

### How USD Generates Prototype Prims {#Usd_ScenegraphInstancing_PrototypeSelection}

To determine the set of prototype prims needed, UsdStage analyzes each prim
marked as instanceable and computes an "instancing key" that consists of:

- The direct composition arcs on the prim that pull in scene description, in 
order from strongest to weakest
- The variant selections applied to the prim
- The \ref Usd_Page_ValueClips "value clips" that affect the prim
- The UsdStageLoadRules from the stage that apply to the prim and its descendants
- The UsdStagePopulationMask from the stage that applies to the prim and its descendants

These elements, along with the \ref Usd_ScenegraphInstancing_Editing "restriction on per-instance overrides" 
of descendant prims and their properties, guarantee that every prim with the 
same key will have the same scenegraph hierarchy beneath them, and that 
computing any values or metadata will return the same result.  

UsdStage groups the instanceable prims by their key and generates a prototype
prim for each group.  During this process, UsdStage will select one instanceable
prim per group to serve as the source for the corresponding prototype prim.  
This selection is non-deterministic; this allows for multi-threaded discovery
of instances with minimal locking, and since all of the prims in a group have
the same scenegraph hierarchy and values, it doesn't matter which is selected.
Enabling \c USD_INSTANCING #TF_DEBUG flag will given some diagnostic information
about this process.

UsdStage will update prototypes as instanceable prims are added or removed, or
whenever an element that is part of the instancing key changes.  For example,
if an instance prim's variant selection is changed, UsdStage will recompute
its key and either assign that instance to an already-existing prototype if
there are existing instances with the same key, or create a new prototype.

### Nested Instancing {#Usd_ScenegraphInstancing_NestedInstancing}

An instanceable prim may have children that are themselves instanceable.  This
"nested instancing" allows consumers to build up large aggregate assets from
smaller ones and use instancing to share as much of the scenegraph as possible,
even between the smaller pieces.  For example, after constructing a parking lot
asset with many instances of a car model, an asset representing a large 
superstore could be created that brings in multiple instances of that parking 
lot.

<center>
<table border="0">
<caption>Example Scene Description and Scenegraph with Nested Instancing</caption>
<tr>
<td valign="top">
\code
### BuyNLarge.usd

#usda 1.0

def "BuyNLarge"
{
    def "ParkingLot_1" (
        instanceable = true
        references = @./ParkingLot.usd@</ParkingLot>
    )
    {
    }

    def "ParkingLot_2" (
        instanceable = true
        references = @./ParkingLot.usd@</ParkingLot>
    )
    {
    }

    # ...

    def "ParkingLot_n" (
        instanceable = true
        references = @./ParkingLot.usd@</ParkingLot>
    )
    {
    }
}
\endcode

\code
### ParkingLot.usd

#usda 1.0

def "ParkingLot"
{
    def "Car_1" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }

    def "Car_2" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }

    # ...

    def "Car_n" (
        instanceable = true
        references = @./Car.usd@</Car>
    )
    {
    }
}
\endcode
</td>
<td valign="middle">
\image html instancing/Nested_Instancing_Example.png 
</td>
</tr>
</table>
</center>

In the above example, USD will generate two prototype prims to accommodate the
instanced ParkingLot and Car prims.  Even though the Car prims are spread out
among the different ParkingLot prims, USD will recognize that they can all 
share a single prototype prim, since they all have the same composition
structure and can share the same scenegraph.

When \ref Usd_ScenegraphInstancing_Prototypes "traversing the scenegraph using prototypes",
it is important to note that, in the nested case, prims in prototypes may also
be instances and requiring accessing the associated prototypes to continue
traversal. In this example, the prototype prim for the ParkingLot prims,
/__Prototype_2, contains instance Car prims that are associated with their
prototype prim, /__Prototype_1.  

When \ref Usd_ScenegraphInstancing_InstanceProxies "working with instance proxies",
nested instancing will be taken into account and resolved as if instancing were
not being used on the stage.

### Flattening {#Usd_ScenegraphInstancing_Flattening}

When flattening a UsdStage into a single layer via its various \ref Usd_stageSerialization "serialization methods",
each prototype prim in the scenegraph will be written out to specially-named
root prim.  These prims will be referenced by the flattened instance prims that
were associated with that prototype.
