# ============================================
# Windows DLL 复制脚本
# ============================================
# 此脚本通过include()调用，在构建时执行，用于复制DLL文件到可执行文件目录
# 需要预先设置变量：DLL_LIST 和 TARGET_DIR

# 检查必需参数（应由调用者设置）
if(NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "Required parameter: TARGET_DIR (should be set by caller)")
endif()

# 处理DLL列表
if(DEFINED DLL_LIST AND DLL_LIST)
    # 统计实际需要复制的文件数
    set(COPIED_COUNT 0)
    set(COPIED_FILES "")
    
    foreach(DLL IN LISTS DLL_LIST)
        if(NOT EXISTS "${DLL}")
            continue()
        endif()
        
        get_filename_component(DLL_NAME "${DLL}" NAME)
        set(TARGET_DLL "${TARGET_DIR}/${DLL_NAME}")
        
        # 检查文件是否需要更新（比较MD5）
        set(NEEDS_COPY FALSE)
        if(EXISTS "${TARGET_DLL}")
            # 目标文件存在，比较内容（MD5哈希）
            file(MD5 "${DLL}" SRC_MD5)
            file(MD5 "${TARGET_DLL}" DST_MD5)
            if(NOT "${SRC_MD5}" STREQUAL "${DST_MD5}")
                set(NEEDS_COPY TRUE)
            endif()
        else()
            # 目标文件不存在，需要复制
            set(NEEDS_COPY TRUE)
        endif()
        
        if(NEEDS_COPY)
            # 复制DLL文件
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "${TARGET_DIR}"
                RESULT_VARIABLE COPY_RESULT
                OUTPUT_QUIET
                ERROR_QUIET
            )
            
            if(COPY_RESULT EQUAL 0)
                math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
                list(APPEND COPIED_FILES "  - ${DLL_NAME}")
            else()
                message(WARNING "Failed to copy DLL: ${DLL}")
            endif()
            
            # 复制对应的.pdb文件（如果存在）
            string(REGEX REPLACE "\\.dll$" ".pdb" PDB_FILE "${DLL}")
            if(EXISTS "${PDB_FILE}")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PDB_FILE}" "${TARGET_DIR}"
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
            endif()
        endif()
    endforeach()
    
    # 只在有实际复制操作时输出详细信息
    if(COPIED_COUNT GREATER 0)
        message(STATUS "Updated ${COPIED_COUNT} runtime DLLs:")
        foreach(FILE IN LISTS COPIED_FILES)
            message(STATUS "${FILE}")
        endforeach()
    endif()
else()
    # 只在第一次或变化时提示
    if(NOT EXISTS "${TARGET_DIR}/.dll_check_done")
        message(STATUS "No runtime DLL dependencies detected (static linking)")
        file(WRITE "${TARGET_DIR}/.dll_check_done" "checked")
    endif()
endif()

