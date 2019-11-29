#pragma once

// Branch prediction hints
#define CPV_LIKELY(expr) __builtin_expect(!!(expr), 1)
#define CPV_UNLIKELY(expr) __builtin_expect(!!(expr), 0)

// convenient macro to tell where is the line that includes this code
// since __func__, __FUNC__, __PRETTY_FUNCTION isn't macro so a helper function is required
#define CPV_CODEINFO cpv::CodeInfo(cpv::joinString("", \
	"[", __FILE__, ":", __LINE__, ":", std::string_view(__PRETTY_FUNCTION__), "]"))

