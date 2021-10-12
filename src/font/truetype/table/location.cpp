#include "font/truetype/truetype.h"
#include "stream/binaryreader.h"



namespace TrueType {

    std::vector<u32> parseGlyphLocationTable(BinaryReader& reader, u32 tableSize, u32 glyphCount, bool longVersion) {

        if(glyphCount == 0) {
            throw LoaderException("Failed to load location table: Glyph count cannot be 0");
        }

        if(tableSize < (glyphCount + 1) * (longVersion ? 4 : 2)) {
            throw LoaderException("Failed to load header table: Stream size too small");
        }

        std::vector<u32> glyphOffsets;
        glyphOffsets.reserve(glyphCount);

        u32 offset = 0;

        auto readOffsets = [&]<bool Long>() {

            using Type = std::conditional_t<Long, u32, u16>;
            
            offset = reader.read<Type>();

            for(u32 i = 0; i < glyphCount; i++) {

                u32 nextOffset = reader.read<Type>() * (Long ? 1 : 2);

                if(nextOffset == offset) {
                    glyphOffsets.push_back(noOutlineGlyphOffset);
                } else {
                    glyphOffsets.push_back(offset);
                }

                offset = nextOffset;

            }

        };

        if(longVersion) {
            readOffsets.template operator()<true>();
        } else {
            readOffsets.template operator()<false>();
        }

#ifdef ARC_FONT_DEBUG
        Log::info("TrueType Loader", "[Location] Loaded %d entries", glyphOffsets.size());
#endif

        return glyphOffsets;

    }

}