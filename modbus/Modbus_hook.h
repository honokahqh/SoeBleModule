#ifndef MODBUS_HOOK_H
#define MODBUS_HOOK_H
#if MBS_FUNCTION_01_ENABLE
void MBS_Function01H(void);
#endif
#if MBS_FUNCTION_02_ENABLE
void MBS_Function02H(void);
#endif
#if MBS_FUNCTION_03_ENABLE
void MBS_Function03H(void);
#endif
#if MBS_FUNCTION_04_ENABLE
void MBS_Function04H(void);
#endif

#if MBS_FUNCTION_05_ENABLE
void MBS_Function05H(void);
#endif
#if MBS_FUNCTION_06_ENABLE
void MBS_Function06H(void);
#endif
#if MBS_FUNCTION_0F_ENABLE
void MBS_Function0FH(void);
#endif
#if MBS_FUNCTION_10_ENABLE
void MBS_Function10H(void);
#endif
#endif /* MODBUS_HOOK_H */
