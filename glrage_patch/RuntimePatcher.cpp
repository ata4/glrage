#include "RuntimePatcher.hpp"
#include "AssaultRigsPatcher.hpp"
#include "TombRaiderPatcher.hpp"
#include "WipeoutPatcher.hpp"

#include <glrage/GLRage.hpp>
#include <glrage_util/ErrorUtils.hpp>
#include <glrage_util/StringUtils.hpp>

#include <Shlwapi.h>
#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <memory>

#include <map>
#include <vector>

namespace glrage {

void RuntimePatcher::patch()
{
    getModulePath();
    getModuleVersion();

    // run known patches
    // clang-format off
    std::map<std::string, std::shared_ptr<RuntimePatch>> patches = {
        {"Tomb Raider",      std::make_shared<TombRaiderPatcher>(false)},
        {"Tomb Raider Gold", std::make_shared<TombRaiderPatcher>(true)},
        {"Assault Rigs",     std::make_shared<AssaultRigsPatcher>()},
        {"Wipeout",          std::make_shared<WipeoutPatcher>()}
    };
    // clang-format on

    Context& ctx = GLRage::getContext();
    ctx.setGameID(m_ctx.fileName);

    // set config path based on the module file name
    m_ctx.config.setPath(
        ctx.getBasePath() + L"\\patches\\" + m_ctx.fileNameW + L".ini");

    // apply patch module as defined in the config
    std::string game = m_ctx.config.getString("game", "");
    auto patch = patches.find(game);
    if (patch == patches.end()) {
        ErrorUtils::error("Invalid patch module '" + game + "'");
    }

    patch->second->setContext(m_ctx);
    patch->second->apply();
}

void RuntimePatcher::getModulePath()
{
    // get executable name
    m_ctx.path.reserve(MAX_PATH);
    GetModuleFileName(nullptr, &m_ctx.path[0], m_ctx.path.capacity());

    // extract file name and copy result
    m_ctx.fileNameW = PathFindFileName(m_ctx.path.c_str());

    // remove extension
    m_ctx.fileNameW =
        m_ctx.fileNameW.substr(0, m_ctx.fileNameW.find_last_of(L"."));

    // convert to lower case
    transform(m_ctx.fileNameW.begin(), m_ctx.fileNameW.end(),
        m_ctx.fileNameW.begin(), ::towlower);

    // convert to UTF-8
    m_ctx.fileName = std::string(StringUtils::wideToUtf8(m_ctx.fileNameW));
}

void RuntimePatcher::getModuleVersion()
{
    DWORD size = GetFileVersionInfoSize(m_ctx.path.c_str(), nullptr);
    if (!size) {
        return;
    }

    std::vector<uint8_t> versionInfo(size);
    if (!GetFileVersionInfo(m_ctx.path.c_str(), 0, size, &versionInfo[0])) {
        return;
    }

    UINT fileInfoLen = 0;
    VS_FIXEDFILEINFO* fileInfoBuf;
    if (VerQueryValue(&versionInfo[0], L"\\",
            reinterpret_cast<LPVOID*>(&fileInfoBuf), &fileInfoLen)) {
        m_ctx.fileInfo = *fileInfoBuf;
    }
}

} // namespace glrage