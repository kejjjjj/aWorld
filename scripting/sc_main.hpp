#pragma once

enum Success : signed char;
namespace Varjus { struct State; }

[[nodiscard]] Success SC_AddWorldObjects(Varjus::State& state);