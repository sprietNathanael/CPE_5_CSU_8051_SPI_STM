;******************************************************************************
;ASM8051_Lib.asm

;   Ce fichier contient des routines Assembleur utiles au TP Microcontrôleur
;   Aucune modification n'est nécessaire.
;   Il suffit d'ajouter ce fichier à votre Projet
;   TARGET MCU  :  C8051F020 

;******************************************************************************

//$INCLUDE(C8051F020.INC)  ; Register definition file.
;******************************************************************************
;Declaration des variables et fonctions publiques
;******************************************************************************

PUBLIC _fct_tempo

;******************************************************************************
;Consignes de segmentation
;******************************************************************************

Timer_UartLib     segment  CODE

               rseg     Timer_UartLib  ; Switch to this code segment.
               using    0              ; Specify register bank for the following
                                       ; program code.

;******************************************************************************
;******************************************************************************
; __tempo
;
; Description: Sous-programme produisant une temporisation logicielle
;              paramétrable par la variable csg_tempo.
;              La temporisation générée est égale à csg_tempo micro-secondes.
;              ATTENTION: csg_tempo ne doit pas être inférieure à 2
;
; Paramètres d'entrée:  csg_tempo dans R6(MSB) et R7(LSB)
; Paramètres de sortie: aucun
; Registres modifiés: R6 et R7
; Pile: 2 octets (sauf pour l'appel de la sous-routine)
;******************************************************************************

_fct_tempo:
         PUSH  ACC
         MOV   A,R5
         PUSH  ACC
         MOV   A,R7
         DEC   R7
         JNZ   ?C0006
         DEC   R6
?C0006:
?C0001:
         MOV   A,R7
         ORL   A,R6
         JZ    ?C0005

         MOV   A,R7
         DEC   R7
         JNZ   ?C0007
         DEC   R6
?C0007:
         MOV   R5,#01H
?C0003:
         MOV   A,R5
         JZ    ?C0001
         DEC   R5
         SJMP  ?C0003
?C0005:
         POP   ACC
         MOV   R5,ACC
         POP   ACC
         RET
;******************************************************************************

end