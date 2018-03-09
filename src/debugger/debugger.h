/*
 * Copyright (C) 2018 Ben Smith
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef BINJGB_DEBUGGER_H__
#define BINJGB_DEBUGGER_H__

#include <array>

#include "imgui.h"
#include "imgui_memory_editor.h"

#include "emulator-debug.h"
#include "host.h"

const ImVec2 kTileSize(8, 8);
const ImVec2 k8x16OBJSize(8, 16);
const ImVec2 kScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
const ImVec2 kTileMapSize(TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
const ImU32 kHighlightColor(IM_COL32(0, 255, 0, 192));
const ImVec4 kPCColor(0.2f, 1.f, 0.1f, 1.f);
const ImVec4 kRegColor(1.f, 0.75f, 0.3f, 1.f);

void SetPaletteAndEnable(Host* host, ImDrawList* draw_list,
                         const PaletteRGBA& palette);
void DisablePalette(Host* host, ImDrawList* draw_list);

class TileImage {
 public:
  TileImage();

  void Init(Host* host);
  void Upload(Emulator*);
  // Return true if hovering on the tile.
  bool DrawTile(ImDrawList* draw_list, int index, const ImVec2& ul_pos,
                f32 scale, PaletteRGBA palette, bool xflip = false,
                bool yflip = false);
  // Return -1 if not hovering, or tile index if hovering.
  int DrawOBJ(ImDrawList* draw_list, ObjSize obj_size, int index,
              const ImVec2& ul_pos, f32 scale, PaletteRGBA palette, bool xflip,
              bool yflip);

 private:
  Host* host;
  TileData tile_data;
  HostTexture* texture;
};

class Debugger {
 public:
  Debugger();
  Debugger(const Debugger&) = delete;
  Debugger& operator=(const Debugger&) = delete;

  ~Debugger();

  bool Init(const char* filename, int audio_frequency, int audio_frames,
            int font_scale, bool paused_at_start);
  void Run();

 private:
  // static
  std::string PrettySize(size_t size);

  void OnAudioBufferFull();
  void OnKeyDown(HostKeycode);
  void OnKeyUp(HostKeycode);

  void StepInstruction();
  void StepFrame();
  void TogglePause();
  void Exit();

  void WriteStateToFile();
  void ReadStateFromFile();

  void SetAudioVolume(f32 volume);

  void MainMenuBar();

  void BeginRewind();
  void EndRewind();
  void BeginAutoRewind();
  void EndAutoRewind();
  void AutoRewind(f64 ms);
  void RewindTo(Cycles cycles);

  EmulatorInit emulator_init;
  HostInit host_init;
  Emulator* e = nullptr;
  Host* host = nullptr;
  const char* save_filename = nullptr;
  const char* save_state_filename = nullptr;
  const char* rom_usage_filename = nullptr;

  enum RunState {
    Exiting,
    Running,
    Paused,
    SteppingFrame,
    SteppingInstruction,
    Rewinding,
    AutoRewinding,
  };
  RunState run_state = Running;

  TileImage tiledata_image;

  f32 audio_volume = 0.5f;

  bool highlight_obj = false;
  int highlight_obj_index = 0;
  bool highlight_tile = false;
  int highlight_tile_index = 0;

  struct Window {
    explicit Window(Debugger* d) : d(d) {}
    Debugger* d;
    bool is_open = true;
  };

  struct AudioWindow : Window {
    explicit AudioWindow(Debugger*);
    void Tick();

    static const int kAudioDataSamples = 1000;
    f32 audio_data[2][kAudioDataSamples] = {};
  };

  struct DisassemblyWindow : Window {
    explicit DisassemblyWindow(Debugger*);
    void Tick();

    // Used to collect disassembled instructions.
    std::array<Address, 65536> instrs;
    int instr_count = 0;
  };

  struct EmulatorWindow : Window {
    explicit EmulatorWindow(Debugger*);
    void Tick();
  };

  struct MapWindow : Window {
    explicit MapWindow(Debugger*);
    void Tick();
  };

  struct MemoryWindow : Window {
    explicit MemoryWindow(Debugger*);
    void Tick();

    MemoryEditor memory_editor;
    Address memory_editor_base = 0;
  };

  struct ObjWindow : Window {
    explicit ObjWindow(Debugger*);
    void Tick();
  };

  struct RewindWindow : Window {
    explicit RewindWindow(Debugger*);
    ~RewindWindow();
    void Tick();

    FileData reverse_step_save_state;
  };

  struct ROMWindow : Window {
    explicit ROMWindow(Debugger*);
    void Init();
    void Tick();

    HostTexture* rom_texture = nullptr;
    int rom_texture_width = 0;
    int rom_texture_height = 0;
  };

  struct TiledataWindow : Window {
    explicit TiledataWindow(Debugger*);
    void Tick();
  };

  AudioWindow audio_window;
  DisassemblyWindow disassembly_window;
  EmulatorWindow emulator_window;
  MapWindow map_window;
  MemoryWindow memory_window;
  ObjWindow obj_window;
  RewindWindow rewind_window;
  ROMWindow rom_window;
  TiledataWindow tiledata_window;
};

#endif  // #define BINJGB_DEBUGGER_H__
