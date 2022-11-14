# MDL

`*.mdl` file is Model file for GoldSrc engine.

Features:
- Skeletal animations
- Skins (texture override)
- Body parts (optional parts of the mesh)

## IDSQ

Model file, must start by 4 characters `IDSQ` followed by version `10` (32-bit integer).

| Magic  | Version | Source                                                      |
|--------|---------|-------------------------------------------------------------|
| `IDPO` | `6`     | [Quake](http://tfc.duke.free.fr/coding/mdl-specs-en.html)   |
| `IDP2` | `8`     | [Quake 2](http://tfc.duke.free.fr/coding/md2-specs-en.html) |
| `IDSQ` | `10`    | GoldSrc                                                     |
| `IDST` | `48`    | [Source](https://developer.valvesoftware.com/wiki/MDL)      |

### Basic Info + Offsets

```cpp
struct CountOffset
{
    // Number of elements
    uint32 Count;
    // Offset from beginning of the file
    uint32 Offset;
};

struct BasicInfo
{
    char Name[64]; ///< Null-terminated, null-padded
    int32 Size;
    
    vec3 EyePosition;
    
    vec3 Min;
    vec3 Max;
    
    // Bounding-box
    vec3 BBMin;
    vec3 BBMax;
    
    uint32 Flags;
    
    CountOffset Bones;
    CountOffset BoneControllers;
    
    CountOffset Hitboxes;
    
    // Animations
    CountOffset Sequences;
    CountOffset SequenceGroups;
    
    CountOffset Textures;
    uint32      TextureDataOffset;
    
    uint32 SkinRefCount;
    uint32 SkinFamilyCount;
    uint32 SkinOffset;
    
    CountOffset BodyParts;
    
    CountOffset Attachments;
    
    CountOffset Sounds;
    CountOffset SoundGroups;
    
    CountOffset Transitions;
};
```

### Bones

Array of 112 byte structs.

```cpp
struct Bone
{
    char  Name[32]; ///< Bone name for symbolic links, null-terminated
    int32 Parent; ///< Parent bone index
    int32 Flags;
    int32 BoneController[6]; ///< Bone controller index, -1 == none
    vec3  Position[2]; ///< Default Degree-of-Freedom values, [0] = position, [1] = rotation
    vec3  Rotation[2]; ///< Scale for delta Degree-of-Freedom values, [0] = position, [1] = rotation
};
static_assert(sizeof(Bone) == 112);
```

### Bone Controllers

Array of 24 byte structs.

```cpp
struct BoneController
{
    int32 BoneIndex; ///< -1 == 0
    int32 Type; ///< X, Y, Z, XR, YR, ZR, M
    float Start;
    float End;
    int32 Rest; ///< byte index value at rest
    int32 Index; ///< 0-3 user set controller, 4 mouth
};
static_assert(sizeof(BoneController) == 24);
```

### Hitboxes

Array of 24 byte structs.

```cpp
/// Intersection boxes
struct BBox
{
    int32 Bone; ///< Attached to which bone
    int32 Group; ///< Intersection group

    // Bounding box
    vec3 BB_Max;
    vec3 BB_Min;
};
static_assert(sizeof(BBox) == 32);
```

### Sequences

```cpp
struct SeqDescription
{
    char Label[32];

    float Fps; ///< frames per second
    int32 Flags; ///< looping/non-looping flags

    int32 Activity;
    int32 ActivityWeight;

    CountOffset Events;

    int32 FrameCount; ///< number of frames per sequence

    CountOffset Pivots; ///< Foot pivots

    int32 MotionType;
    int32 MotionBone;
    vec3  LinearMovement;
    int32 AutomovePosIndex;
    int32 AutomoveAngleIndex;

    // per sequence bounding box
    vec3 BB_Min;
    vec3 BB_Max;

    int32 BlendCount;
    int32 AnimationOffset; ///< mstudioanim_t pointer relative to start of sequence group data
    // [blend][bone][X, Y, Z, XR, YR, ZR]

    int32 BlendType[2]; ///< X, Y, Z, XR, YR, ZR
    float BlendStart[2]; ///< starting value
    float BlendEnd[2]; ///< ending value
    int32 BlendParent;

    int32 SeqGroup; ///< sequence group for demand loading

    int32 EntryNode; ///< transition node at entry
    int32 ExitNode; ///< transition node at exit
    int32 NodeFlags; ///< transition rules

    int32 NextSeq; ///< auto advancing sequences
};
static_assert(sizeof(SeqDescription) == 176);
```

#### Events

See `SeqDescription.Events`, array of 76 byte structs.

```cpp
struct Event
{
    int Frame;
    int Event;
    int Type;
    char Options[64];
};
static_assert(sizeof(Event) == 76);
```

#### Pivots

See `SeqDescription.Pivots`.

///TODO

```cpp
struct Pivot
{
    vec3 Origin; ///< pivot point
    int  Start;
    int  End;
};
```

#### Blends

//TODO

### Sequence Groups

Array of 104 byte structs.

```cpp
struct SeqGroup
{
    char Label[32]; ///< textual name
    char Name[64]; ///< file name
    int  Unused1; ///< was "cache"  - index pointer
    int  Unused2; ///< was "data" -  hack for group 0
};
static_assert(sizeof(SeqGroup) == 104);
```

### Textures

```cpp
enum TextureFlag : int32
{
    Chrome      = 1 << 1 /* 2 */, ///< Used for Baby Headcrab
    Additive    = 1 << 5 /* 32 */,
    Transparent = 1 << 6 /* 64 */
};

struct Texture
{
    char        Name[64]; ///< Null-terminated
    TextureFlag Flags;
    int32       Width;
    int32       Height;
    int32       DataOffset; ///< Relative to the beginning of the file
};
static_assert(sizeof(Texture) == 80);
```

At the `Texture.DataOffset` there are `texture.Width * texture.Height` bytes (indices) followed by `256` RGB `uint8` palette.

### Skin Ref / Family



## IDST
