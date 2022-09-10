# Forge Game Data

FGD file is definition of entities.

This documentation is based on [Official FGD definition](https://developer.valvesoftware.com/wiki/FGD) with some testing.
Upgraded and corrected version for some official games can be found at [Pinsplash/SEFGD](https://github.com/Pinsplash/SEFGD).

Version below is **Source** variant of the specification, features missing in **GoldSrc** are marked.
**Source 2** is mostly ignored.

Official FGD files use mostly `\t` for spacing but spaces are also sometimes present.

All text outside quotes seems to be case-insensitive.

There does not seem to be a way to use `"` character inside quoted text.
`"\n"` is officially supported only inside [class](#classes) description as 2 characters (not a single CR or LF character).
All quoted texts can be made from multiple quoted texts joined together using `+` (useful for new text wrapping) like `"sample" + " text"`.
Official implementation does not support newline inside quoted text (starting `"` on different line than ending `"`).

## Comments

Comments are lines starting with `//` (spaces and tabs before it are ignored).

There are no block comments.

## Utility

### Map size
Define size of map allowed in the editor.
Maximum value for `vbsp` is `-16384, 16384`.

```@mapsize(-16384, 16384)```

### Include other file

Acts as `#include` in C/C++ and can be nested.

```@include "path_to_file.fgd"```

### Material Exclusion

These lists define paths that Material Browser will not use when presenting you with a palette of textures to choose from.
It should have no effect on what files are actually available to a map.

```
@MaterialExclusion
[
    // Names of the sub-directories we don't want to load materials from
    "debug"
    "engine"
    "hud"
    "vgui"
]
```

### Auto Vis Group

This permits customizing the automatic Visgroups tab of the Filter Control toolbar.
The first title is the name of the "parent," and the next is the "children."
Finally, comes a list of entity classes that will be placed in the visgroup.

If the parent already exists, the new entry will be merged with the previous ones (including the default list of groups).
This permits creating trees with multiple levels of grouping.
If a visgroup becomes entirely empty, it will not appear in the list.

You may add entities to existing groups if the "Parent" of the autovisgroup is the name of an existing group, like "World Details".
For example, you could add `func_brush` to the list "World Details".

```
@AutoVisGroup = "Brushes"
[
    "Triggers"
    [
        "trigger_once"
        "trigger_multiple"
    ]
    "Tool Brushes"
    [
        "func_areaportal"
        "func_viscluster"
    ] 
]

@AutoVisGroup = "Tool Brushes"
[
    "Vis Clusters"
    [
        "func_viscluster"
    ]
]
```

## Classes

```
@<class_type> [options...] = [name]
@<class_type> [options...] = [name] : "description"
```
Options are optional and when present must be separated by at least a space or a tab character.

### Class Type

| Name             | Human name   | GoldSrc | Source | Usage                                         |
|------------------|--------------|:-------:|:------:|-----------------------------------------------|
| `@BaseClass`     | Base Class   |    ✓    |   ✓    | Same as `interface` in programming languages. |
| `@PointClass`    | Point Entity |    ✓    |   ✓    | Positioned entity.                            |
| `@SolidClass`    | Brush Entity |    ✓    |   ✓    | Entity made from brushes (area).              |
| `@NPCClass`      | NPC          |         |   ✓    |                                               |
| `@KeyFrameClass` | Key Frame    |         |   ✓    |                                               |
| `@MoveClass`     | Track        |         |   ✓    | Track to be automatically linked.             |
| `@FilterClass`   | Filter       |         |   ✓    |                                               |

### Options

There may be multiple of same options on one class.

| Code                                                                  | GoldSrc | Source | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
|-----------------------------------------------------------------------|:-------:|:------:|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `axis( <property> )`                                                  |         |        | Allows positioning two points joined by a line in the map. The property value is set to `"x1 y1 z1, x2 y2 z2"` by default.                                                                                                                                                                                                                                                                                                                                                                           |
| `base( <BaseClass>, <BaseClass>, ... )`                               |    ✓    |   ✓    | This lets you attach previously defined `BaseClass`es (see above) to an entity. You can specify multiple `BaseClass`es, each separated by a comma.                                                                                                                                                                                                                                                                                                                                                   |
| `bbox( <vec_min>, <vec_max> )`                                        |         |        | Sets the size of the entity's bounding box.                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `color( <r> <g> <b> )`                                                |    ✓    |   ✓    | Each color components must be value of a byte. Default color is magenta.                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `cylinder( <xxx> )`                                                   |         |   ✓    | Draw a cylinder between two entities. This is similar to `line()`, but with the addition of two `radius` properties that are looked up on the target entities. These define the size of the start and end of the cylinder.                                                                                                                                                                                                                                                                           |
| `frustum( <fov>, <near>, <far>, <color>, <pitch_scale> )`             |         |   ✓    | Creates a rectangular cone extending from the entity. FOV defines the spread angle (0-180). Near and far define at what distances will be highlighted. The color value defines what color the cone will be shown with. Pitch_scale allows inverting the pitch angle when rendering the cone. The first four values must be property names, the last is a literal. If not specified, values are taken from _`fov`, `_nearplane`, `_farplane`, and `_light`, respectively. `pitch_scale` is set to -1. |
| `halfgridsnap`                                                        |         |   ✓    | When moving this entity, it will snap to half the current grid size. This is somewhat special as it takes no arguments or parentheses.                                                                                                                                                                                                                                                                                                                                                               |
| `iconsprite( "path/sprite.vmt" )`                                     |    ✓    |   ✓    | If this is used, the specified sprite will be shown in the Hammer 3D view instead of a flat-shaded colored box. This will work along-side the `studio()` or `studioprop()` commands. If no sprite name is set, it uses the `model` property. `scale`, `rendermode`, `_light` and `angles` properties affect the sprite.                                                                                                                                                                              |
| `lightcone( <inner_fov>, <outer_fov>, <color>, <pitch_scale> )`       |         |   ✓    | Renders the cone used on `light_spot` entities. `inner_fov` is the key for the innermost cone section, `outer_fov` is the outermost. `pitch_scale` allows inverting the pitch angle when rendering the cone. Values are taken from `_inner_cone`, `_cone`, and `_light`, respectively, if they aren't specified. This reads many other values corresponding to `light_spot` properties.                                                                                                              |
| `lightprop( "path/model.mdl" )`                                       |         |   ✓    | Identical to `studioprop()`, except that the pitch of the model is inverted.                                                                                                                                                                                                                                                                                                                                                                                                                         |
| `line( <color>, <start_key>, <start_value>, <end_key>, <end_value> )` |         |   ✓    | Draws a line between two entities. The `value` properties in this entity give the names to look for in the `key` property on other entities. `key` is usually set to `targetname`. The color sets the color of the line when the entity is not selected. The second entity defaults to this one if not set.                                                                                                                                                                                          |
| `obb( <vec_min>, <vec_max> )`                                         |         |   ✓    | Identical to `bbox` but oriented to the entity's angles.                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `origin( <property> )`                                                |         |   ✓    | Allows positioning a vector property in the map.                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| `sidelist( <sides> )`                                                 |         |   ✓    | Highlight brush faces listed in the given property (as a space-seperated ID list). If not specified, the property used is `sides`.                                                                                                                                                                                                                                                                                                                                                                   |
| `size( -x,-y,-z,+x,+y,+z )`                                           |    ✓    |   ✓    | Defines the size of the default cube used when no model or sprite is specified.                                                                                                                                                                                                                                                                                                                                                                                                                      |
| `sphere( <propertyname> )`                                            |    ✓    |   ✓    | If an entity has a radius of effect, like a sound for example, a sphere will be displayed in Hammer's 2D and 3D views. You need to specify the property that will control the sphere size. If no property is specified, it will look for a `radius` property.                                                                                                                                                                                                                                        |
| `studio( "path/model.mdl" )`                                          |    ✓    |   ✓    | Identical to `studioprop()`, but the bounding box around the entity will ignore this model. This is useful for entities that don't render the model ingame.                                                                                                                                                                                                                                                                                                                                          |
| `studioprop( "path/model.mdl" )`                                      |         |   ✓    | If this is used, the entity will be displayed in the 3D view as the specified model. If no model is specified, the value of the entity's model property will be used, if available (Unless hardcoded). Multiple models can be defined.                                                                                                                                                                                                                                                               |
| `vecline( <property> )`                                               |         |        | Allows positioning a vector property in the map. This also draws a line from the entity to the position.                                                                                                                                                                                                                                                                                                                                                                                             |
| `wirebox( <vec_min>, <vec_max> )`                                     |         |   ✓    | Draws a bounding box for two properties. `origin()` helpers should be defined as well to allow moving the points.                                                                                                                                                                                                                                                                                                                                                                                    |
| `worldtext()`                                                         |         |   ✓    | Displays the contents of the `message` keyvalue in the 3D viewport.                                                                                                                                                                                                                                                                                                                                                                                                                                  |

`studioprop`
- If you have an entity with the "angles" property that you want to be able to rotate in Hammer using the mouse (as opposed to only through property editing), you may need to add this modifier.
- The appearance is affected by the `skin` and `rendercolor` properties, similar to prop_dynamic.

#### Helpers

| Code                   | GoldSrc | Source | Description                                                                                                                                                                                                                           |
|------------------------|:-------:|:------:|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `decal()`              |    ✓    |   ✓    | Renders decals on nearby surfaces. This uses the **texture** property to set the material to use.                                                                                                                                     |
| `overlay()`            |         |   ✓    | Renders overlays on a surface. (For `info_overlay`)                                                                                                                                                                                   |
| `overlay_transition()` |         |   ✓    | Renders overlays on the intersections between water and the shore. (For `info_overlay_transition`)                                                                                                                                    |
| `light()`              |         |   ✓    | Present on `light`; its use is unknown.                                                                                                                                                                                               |
| `sprite()`             |    ✓    |   ✓    | Renders the sprite material specified in the model keyvalue (`env_sprite` and variants). For entity icons, use `iconsprite`.                                                                                                          |
| `sweptplayerhull()`    |         |   ✓    | Draws 32x32x72-sized rectangular prisms at two points (`point0` and `point1`), then links corners to show the space needed for one rectangle to move to the other's position. This also adds `origin()` helpers for those properties. |
| `instance()`           |         |   ✓    | Renders the `instance` in the map. It also generates additional properties dynamically corresponding to the instance parameters.                                                                                                      |
| `quadbounds()`         |         |   ✓    | Used for `func_breakable_surf`. Automatically sets the 4 corners of the textured face on save.                                                                                                                                        |

**Source 2** adds `deprecated()` helper which can be useful while **Source** just adds ` (DEPRECATED)` at the end of entity name.

## Properties

```
<codename>(<type>) [options...]
<codename>(<type>) [options...] : "displayname"
<codename>(<type>) [options...] : "displayname" : <default>
<codename>(<type>) [options...] : "displayname" : <default> : "description"
<codename>(<type>) [options...] : "displayname" :: "description"
```

The only known option is `readonly`.

### Basic Types

| Property  | Example | GoldSrc | Source | Note                                                                           |
|-----------|---------|:-------:|:------:|--------------------------------------------------------------------------------|
| `string`  | `"abc"` |    ✓    |   ✓    | Always quoted using `"`.                                                       |
| `integer` | `1`     |    ✓    |   ✓    |                                                                                |
| `float`   | `"1.5"` |         |   ✓    | Always quoted using `"`. Not available in GoldSrc.                             |
| `boolean` | `1`     |         |   ✓    | Either `0` or `1`.                                                             |
| `flags`   |         |    ✓    |   ✓    | Default value is stored at each flag. Stored as numeric value.                 |
| `choices` |         |    ✓    |   ✓    | One value out of multiple options. Type can be `string`, `integer` or `float`. |

#### Flags

Valid flags are defined in `= [` `]`, one per line.

```
<flagmask> : "displayname" : <default>
```

`<flagmask>` is a doubling sequence `1, 2, 4, 8, 16, 32, 64...` (powers of 2).
If you know index of the sequence, you can use `1 << index` (`<<` = binary shift left) where index starts at `0`.

`<default>` is either `0` or `1`, used as `boolean` value.

[Valve Hammer Editor](https://developer.valvesoftware.com/wiki/Valve_Hammer_Editor) will only display resulting numeric value unless the codename is `spawnflags` in which case it will be used as labels on 2nd tab of properties window.

#### Choices

```
<value> : "displayname"
```

Only one value can be selected.
`<value>` can be `string`, `integer` or `float` and can be combined withing one property.

### Complex Types

| Property               | Example                                                             | GoldSrc | Source | Description                                                                                                                                                                                                               |
|------------------------|---------------------------------------------------------------------|:-------:|:------:|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `angle`                | `"0 0 0"`                                                           |         |   ✓    | Pitch, Yaw, Roll. Adds an angle widget for this property to the entity dialog UI.                                                                                                                                         |
| `angle_negative_pitch` | `-90`                                                               |         |   ✓    | Identical to `angle`, except the pitch is inverted.                                                                                                                                                                       |
| `axis`                 |                                                                     |         |   ✓    | Adds a relative 2-point axis helper.                                                                                                                                                                                      |
| `color255`             | `"0 0 0"` or `"0 0 0 0"`                                            |    ✓    |   ✓    | RGB color, optionally followed by brightness. Value between `0` and `255`. Brightness is an `integer`.                                                                                                                    |
| `color1`               | `"0 0 0"` or `"0 0 0 0"`                                            |         |   ✓    | RGB color, optionally followed by brightness. Value between `0.0` and `1.0`. Brightness is an `integer`.                                                                                                                  |
| `decal`                | `"decals/xxx.spr"`                                                  |    ✓    |   ✓    | Identical to `material`, except it will automatically replace your search filter with `decals/` when opening the material browser. Sometimes, the material you want will be in `overlays/` or in another folder entirely. |
| `filterclass`          |                                                                     |         |   ✓    | Marks property as being the name of the filter to use.                                                                                                                                                                    |
| `instance_file`        |                                                                     |         |   ✓    | Browse for instance files.                                                                                                                                                                                                |
| `instance_parm`        |                                                                     |         |   ✓    | Used in `func_instance_parms` to define fixup variables.                                                                                                                                                                  |
| `instance_variable`    |                                                                     |         |   ✓    | Used in `func_instance` to set fixup variables.                                                                                                                                                                           |
| `material`             | `"effects/flashlight001"` or `"sprites/obj_icons/icon_obj_neutral"` |         |   ✓    | Adds a button that brings up the material browser.                                                                                                                                                                        |
| `node_dest`            | `-1`                                                                |         |   ✓    | Target node.                                                                                                                                                                                                              |
| `node_id`              |                                                                     |         |   ✓    | On nodes, this is used for the Node ID keyvalue to automatically increment it with each consecutive node placed.                                                                                                          |
| `npcclass`             | `"npc_zombie"`                                                      |         |   ✓    | Adds a drop-down selection list populated by entities of the NPCClass type.                                                                                                                                               |
| `origin`               | `"0 0 0"`                                                           |         |   ✓    |                                                                                                                                                                                                                           |
| `particlesystem`       | `"ins_ammo_explosion"`                                              |         |   ✓    | Opens particle browser, Hammer cannot read files from VPKs.                                                                                                                                                               |
| `pointentityclass`     | `"item_dynamic_resupply"`                                           |         |   ✓    | List of all `@PointClass` entities.                                                                                                                                                                                       |
| `scene`                |                                                                     |         |   ✓    | Sound browser to browse scene files.                                                                                                                                                                                      |
| `script`               |                                                                     |         |   ✓    | File browser to browse for VScripts.                                                                                                                                                                                      |
| `scriptlist`           |                                                                     |         |   ✓    | File browser to browse for multiple VScripts.                                                                                                                                                                             |
| `sidelist`             |                                                                     |         |   ✓    | One or more sides.                                                                                                                                                                                                        |
| `sound`                | `"ambient/objects/cannister_loop.wav"`                              |    ✓    |   ✓    | Sound browser for soundscripts or raw sounds.                                                                                                                                                                             |
| `sprite`               | `"sprites/glow01.spr"`                                              |    ✓    |   ✓    | Identical to `material`, except it will automatically replace your search filter with `sprites/` when opening the material browser, and it will add `.vmt` to the end of the material name.                               |
| `studio`               | `"models/props_industrial/barrel_fuel.mdl"`                         |    ✓    |   ✓    | Model browser.                                                                                                                                                                                                            |
| `target_destination`   |                                                                     |    ✓    |   ✓    | Marks property as another entity's `targetname`.                                                                                                                                                                          |
| `target_name_or_class` |                                                                     |         |   ✓    | Marks property as another entity's `targetname` or `classname`.                                                                                                                                                           |
| `target_source`        |                                                                     |    ✓    |   ✓    | Marks property as being the name that other entities may target.                                                                                                                                                          |
| `vecline`              | `"0 0 0"`                                                           |         |   ✓    | Adds an absolute 1-point axis helper, similar to the origin marker.                                                                                                                                                       |
| `vector`               | `"0 0 0"`                                                           |         |   ✓    | 3D vector property.                                                                                                                                                                                                       |

## Inputs & Outputs

[Official documentation on Inputs and Outputs](https://developer.valvesoftware.com/wiki/Inputs_and_Outputs)

Both `input` and `output` support single parameter.

### Inputs

```
input <name>(<param>) : "comment"
```
For no `<param>` use `void`.

```
input Enable(void) : "Makes the entity active."
input SetMaxPieces(integer) : "Sets Max Gib Count."
```

### Outputs

```
output <name>(<param>) : "comment"
```
For no `<param>` use `void`.

```
output OnUser1(void) : "Fires in response to the FireUser1 input."
output OnHealthChanged(float) : "Fired whenever the health of the prop has increased or decreased. Automatically puts the new health amount as a decimal percent (e.g. 45% = 0.45) as the input parameter, unless overridden by the mapper."
```
