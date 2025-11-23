#pragma once

#ifdef __cpp_exceptions
  #define LAB_TRY try
  #define LAB_CATCH(exception) catch (exception)
  #define LAB_PROPAGATE_EXCEPTION throw
#else
  #define LAB_TRY if constexpr (true)
  #define LAB_CATCH(exception) if constexpr (false)
  #define LAB_PROPAGATE_EXCEPTION \
    (void) int { }
#endif
