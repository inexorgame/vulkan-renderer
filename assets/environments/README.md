Additional environment cube maps put here must be 
- Stored as Khronos texture format files (.ktx)
- Cube maps (with mip maps)
- 16 bit HDR RGBA signed float (VK_FORMAT_R16G16B16A16_SFLOAT)

Converting hdr environment maps can be done with [cmft](https://github.com/dariomanesku/cmft) like this:

```cmft --input "papermill.hdr" --filter none --outputNum 1 --output0 "papermill_hdr16f_cube" --output0params ktx,rgba16f,cubemap --generateMipChain true --dstFaceSize 512```

