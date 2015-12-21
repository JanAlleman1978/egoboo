#pragma once

#include "Tool.hpp"

namespace Tools {
/**
 * @brief Convert paletted images to non-paletted images.
 */
struct ConvertPaletted : public Editor::Tool {
public:
    /**
     * @brief Construct this tool.
     */
    ConvertPaletted();

    /**
     * @brief Destruct this tool.
     */
    virtual ~ConvertPaletted();

    /**
     * @brief Run this tool with the specified arguments.
     * @param argument the arguments
     */
    void run(const std::vector<std::shared_ptr<CommandLine::Option>>& arguments) override;

private:
    void convert(const std::string &fileName);

}; // struct ConvertPaletted

struct ConvertPalettedFactory : Editor::ToolFactory {
    Editor::Tool *create() noexcept override {
        try {
            return new ConvertPaletted();
        } catch (...) {
            return nullptr;
        }
    }
}; // struct ConvertPalettedFactory

} // namespace Tools
