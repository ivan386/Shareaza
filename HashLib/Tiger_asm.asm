; #####################################################################################################################
;
; Tiger_asm.asm
;
; Copyright (c) Shareaza Development Team, 2002-2007.
; This file is part of SHAREAZA (shareaza.sourceforge.net)
;
; Shareaza is free software; you can redistribute it
; and/or modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2 of
; the License, or (at your option) any later version.
;
; Shareaza is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with Shareaza; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
; #####################################################################################################################
;
; Tiger_asm - Implementation of Tiger for x86 - use together with TigerTree.cppmp86p
;
; #####################################################################################################################

                        .586p
                        .model      flat, stdcall
                        option      casemap:none                    ; case sensitive
                        option      prologue:none                   ; we generate our own entry/exit code
                        option      epilogue:none

; #####################################################################################################################

m_nState0               equ         0
m_nState1               equ         8
m_nState2               equ         16

                        .data

                        ALIGN       16

T1                      dq          002AAB17CF7E90C5EH   ;   0
                        dq          0AC424B03E243A8ECH   ;   1
                        dq          072CD5BE30DD5FCD3H   ;   2
                        dq          06D019B93F6F97F3AH   ;   3
                        dq          0CD9978FFD21F9193H   ;   4
                        dq          07573A1C9708029E2H   ;   5
                        dq          0B164326B922A83C3H   ;   6
                        dq          046883EEE04915870H   ;   7
                        dq          0EAACE3057103ECE6H   ;   8
                        dq          0C54169B808A3535CH   ;   9
                        dq          04CE754918DDEC47CH   ;  10
                        dq          00AA2F4DFDC0DF40CH   ;  11
                        dq          010B76F18A74DBEFAH   ;  12
                        dq          0C6CCB6235AD1AB6AH   ;  13
                        dq          013726121572FE2FFH   ;  14
                        dq          01A488C6F199D921EH   ;  15
                        dq          04BC9F9F4DA0007CAH   ;  16
                        dq          026F5E6F6E85241C7H   ;  17
                        dq          0859079DBEA5947B6H   ;  18
                        dq          04F1885C5C99E8C92H   ;  19
                        dq          0D78E761EA96F864BH   ;  20
                        dq          08E36428C52B5C17DH   ;  21
                        dq          069CF6827373063C1H   ;  22
                        dq          0B607C93D9BB4C56EH   ;  23
                        dq          07D820E760E76B5EAH   ;  24
                        dq          0645C9CC6F07FDC42H   ;  25
                        dq          0BF38A078243342E0H   ;  26
                        dq          05F6B343C9D2E7D04H   ;  27
                        dq          0F2C28AEB600B0EC6H   ;  28
                        dq          06C0ED85F7254BCACH   ;  29
                        dq          071592281A4DB4FE5H   ;  30
                        dq          01967FA69CE0FED9FH   ;  31
                        dq          0FD5293F8B96545DBH   ;  32
                        dq          0C879E9D7F2A7600BH   ;  33
                        dq          0860248920193194EH   ;  34
                        dq          0A4F9533B2D9CC0B3H   ;  35
                        dq          09053836C15957613H   ;  36
                        dq          0DB6DCF8AFC357BF1H   ;  37
                        dq          018BEEA7A7A370F57H   ;  38
                        dq          0037117CA50B99066H   ;  39
                        dq          06AB30A9774424A35H   ;  40
                        dq          0F4E92F02E325249BH   ;  41
                        dq          07739DB07061CCAE1H   ;  42
                        dq          0D8F3B49CECA42A05H   ;  43
                        dq          0BD56BE3F51382F73H   ;  44
                        dq          045FAED5843B0BB28H   ;  45
                        dq          01C813D5C11BF1F83H   ;  46
                        dq          08AF0E4B6D75FA169H   ;  47
                        dq          033EE18A487AD9999H   ;  48
                        dq          03C26E8EAB1C94410H   ;  49
                        dq          0B510102BC0A822F9H   ;  50
                        dq          0141EEF310CE6123BH   ;  51
                        dq          0FC65B90059DDB154H   ;  52
                        dq          0E0158640C5E0E607H   ;  53
                        dq          0884E079826C3A3CFH   ;  54
                        dq          0930D0D9523C535FDH   ;  55
                        dq          035638D754E9A2B00H   ;  56
                        dq          04085FCCF40469DD5H   ;  57
                        dq          0C4B17AD28BE23A4CH   ;  58
                        dq          0CAB2F0FC6A3E6A2EH   ;  59
                        dq          02860971A6B943FCDH   ;  60
                        dq          03DDE6EE212E30446H   ;  61
                        dq          06222F32AE01765AEH   ;  62
                        dq          05D550BB5478308FEH   ;  63
                        dq          0A9EFA98DA0EDA22AH   ;  64
                        dq          0C351A71686C40DA7H   ;  65
                        dq          01105586D9C867C84H   ;  66
                        dq          0DCFFEE85FDA22853H   ;  67
                        dq          0CCFBD0262C5EEF76H   ;  68
                        dq          0BAF294CB8990D201H   ;  69
                        dq          0E69464F52AFAD975H   ;  70
                        dq          094B013AFDF133E14H   ;  71
                        dq          006A7D1A32823C958H   ;  72
                        dq          06F95FE5130F61119H   ;  73
                        dq          0D92AB34E462C06C0H   ;  74
                        dq          0ED7BDE33887C71D2H   ;  75
                        dq          079746D6E6518393EH   ;  76
                        dq          05BA419385D713329H   ;  77
                        dq          07C1BA6B948A97564H   ;  78
                        dq          031987C197BFDAC67H   ;  79
                        dq          0DE6C23C44B053D02H   ;  80
                        dq          0581C49FED002D64DH   ;  81
                        dq          0DD474D6338261571H   ;  82
                        dq          0AA4546C3E473D062H   ;  83
                        dq          0928FCE349455F860H   ;  84
                        dq          048161BBACAAB94D9H   ;  85
                        dq          063912430770E6F68H   ;  86
                        dq          06EC8A5E602C6641CH   ;  87
                        dq          087282515337DDD2BH   ;  88
                        dq          02CDA6B42034B701BH   ;  89
                        dq          0B03D37C181CB096DH   ;  90
                        dq          0E108438266C71C6FH   ;  91
                        dq          02B3180C7EB51B255H   ;  92
                        dq          0DF92B82F96C08BBCH   ;  93
                        dq          05C68C8C0A632F3BAH   ;  94
                        dq          05504CC861C3D0556H   ;  95
                        dq          0ABBFA4E55FB26B8FH   ;  96
                        dq          041848B0AB3BACEB4H   ;  97
                        dq          0B334A273AA445D32H   ;  98
                        dq          0BCA696F0A85AD881H   ;  99
                        dq          024F6EC65B528D56CH   ; 100
                        dq          00CE1512E90F4524AH   ; 101
                        dq          04E9DD79D5506D35AH   ; 102
                        dq          0258905FAC6CE9779H   ; 103
                        dq          02019295B3E109B33H   ; 104
                        dq          0F8A9478B73A054CCH   ; 105
                        dq          02924F2F934417EB0H   ; 106
                        dq          03993357D536D1BC4H   ; 107
                        dq          038A81AC21DB6FF8BH   ; 108
                        dq          047C4FBF17D6016BFH   ; 109
                        dq          01E0FAADD7667E3F5H   ; 110
                        dq          07ABCFF62938BEB96H   ; 111
                        dq          0A78DAD948FC179C9H   ; 112
                        dq          08F1F98B72911E50DH   ; 113
                        dq          061E48EAE27121A91H   ; 114
                        dq          04D62F7AD31859808H   ; 115
                        dq          0ECEBA345EF5CEAEBH   ; 116
                        dq          0F5CEB25EBC9684CEH   ; 117
                        dq          0F633E20CB7F76221H   ; 118
                        dq          0A32CDF06AB8293E4H   ; 119
                        dq          0985A202CA5EE2CA4H   ; 120
                        dq          0CF0B8447CC8A8FB1H   ; 121
                        dq          09F765244979859A3H   ; 122
                        dq          0A8D516B1A1240017H   ; 123
                        dq          00BD7BA3EBB5DC726H   ; 124
                        dq          0E54BCA55B86ADB39H   ; 125
                        dq          01D7A3AFD6C478063H   ; 126
                        dq          0519EC608E7669EDDH   ; 127
                        dq          00E5715A2D149AA23H   ; 128
                        dq          0177D4571848FF194H   ; 129
                        dq          0EEB55F3241014C22H   ; 130
                        dq          00F5E5CA13A6E2EC2H   ; 131
                        dq          08029927B75F5C361H   ; 132
                        dq          0AD139FABC3D6E436H   ; 133
                        dq          00D5DF1A94CCF402FH   ; 134
                        dq          03E8BD948BEA5DFC8H   ; 135
                        dq          0A5A0D357BD3FF77EH   ; 136
                        dq          0A2D12E251F74F645H   ; 137
                        dq          066FD9E525E81A082H   ; 138
                        dq          02E0C90CE7F687A49H   ; 139
                        dq          0C2E8BCBEBA973BC5H   ; 140
                        dq          0000001BCE509745FH   ; 141
                        dq          0423777BBE6DAB3D6H   ; 142
                        dq          0D1661C7EAEF06EB5H   ; 143
                        dq          0A1781F354DAACFD8H   ; 144
                        dq          02D11284A2B16AFFCH   ; 145
                        dq          0F1FC4F67FA891D1FH   ; 146
                        dq          073ECC25DCB920ADAH   ; 147
                        dq          0AE610C22C2A12651H   ; 148
                        dq          096E0A810D356B78AH   ; 149
                        dq          05A9A381F2FE7870FH   ; 150
                        dq          0D5AD62EDE94E5530H   ; 151
                        dq          0D225E5E8368D1427H   ; 152
                        dq          065977B70C7AF4631H   ; 153
                        dq          099F889B2DE39D74FH   ; 154
                        dq          0233F30BF54E1D143H   ; 155
                        dq          09A9675D3D9A63C97H   ; 156
                        dq          05470554FF334F9A8H   ; 157
                        dq          0166ACB744A4F5688H   ; 158
                        dq          070C74CAAB2E4AEADH   ; 159
                        dq          0F0D091646F294D12H   ; 160
                        dq          057B82A89684031D1H   ; 161
                        dq          0EFD95A5A61BE0B6BH   ; 162
                        dq          02FBD12E969F2F29AH   ; 163
                        dq          09BD37013FEFF9FE8H   ; 164
                        dq          03F9B0404D6085A06H   ; 165
                        dq          04940C1F3166CFE15H   ; 166
                        dq          009542C4DCDF3DEFBH   ; 167
                        dq          0B4C5218385CD5CE3H   ; 168
                        dq          0C935B7DC4462A641H   ; 169
                        dq          03417F8A68ED3B63FH   ; 170
                        dq          0B80959295B215B40H   ; 171
                        dq          0F99CDAEF3B8C8572H   ; 172
                        dq          0018C0614F8FCB95DH   ; 173
                        dq          01B14ACCD1A3ACDF3H   ; 174
                        dq          084D471F200BB732DH   ; 175
                        dq          0C1A3110E95E8DA16H   ; 176
                        dq          0430A7220BF1A82B8H   ; 177
                        dq          0B77E090D39DF210EH   ; 178
                        dq          05EF4BD9F3CD05E9DH   ; 179
                        dq          09D4FF6DA7E57A444H   ; 180
                        dq          0DA1D60E183D4A5F8H   ; 181
                        dq          0B287C38417998E47H   ; 182
                        dq          0FE3EDC121BB31886H   ; 183
                        dq          0C7FE3CCC980CCBEFH   ; 184
                        dq          0E46FB590189BFD03H   ; 185
                        dq          03732FD469A4C57DCH   ; 186
                        dq          07EF700A07CF1AD65H   ; 187
                        dq          059C64468A31D8859H   ; 188
                        dq          0762FB0B4D45B61F6H   ; 189
                        dq          0155BAED099047718H   ; 190
                        dq          068755E4C3D50BAA6H   ; 191
                        dq          0E9214E7F22D8B4DFH   ; 192
                        dq          02ADDBF532EAC95F4H   ; 193
                        dq          032AE3909B4BD0109H   ; 194
                        dq          0834DF537B08E3450H   ; 195
                        dq          0FA209DA84220728DH   ; 196
                        dq          09E691D9B9EFE23F7H   ; 197
                        dq          00446D288C4AE8D7FH   ; 198
                        dq          07B4CC524E169785BH   ; 199
                        dq          021D87F0135CA1385H   ; 200
                        dq          0CEBB400F137B8AA5H   ; 201
                        dq          0272E2B66580796BEH   ; 202
                        dq          03612264125C2B0DEH   ; 203
                        dq          0057702BDAD1EFBB2H   ; 204
                        dq          0D4BABB8EACF84BE9H   ; 205
                        dq          091583139641BC67BH   ; 206
                        dq          08BDC2DE08036E024H   ; 207
                        dq          0603C8156F49F68EDH   ; 208
                        dq          0F7D236F7DBEF5111H   ; 209
                        dq          09727C4598AD21E80H   ; 210
                        dq          0A08A0896670A5FD7H   ; 211
                        dq          0CB4A8F4309EBA9CBH   ; 212
                        dq          081AF564B0F7036A1H   ; 213
                        dq          0C0B99AA778199ABDH   ; 214
                        dq          0959F1EC83FC8E952H   ; 215
                        dq          08C505077794A81B9H   ; 216
                        dq          03ACAAF8F056338F0H   ; 217
                        dq          007B43F50627A6778H   ; 218
                        dq          04A44AB49F5ECCC77H   ; 219
                        dq          03BC3D6E4B679EE98H   ; 220
                        dq          09CC0D4D1CF14108CH   ; 221
                        dq          04406C00B206BC8A0H   ; 222
                        dq          082A18854C8D72D89H   ; 223
                        dq          067E366B35C3C432CH   ; 224
                        dq          0B923DD61102B37F2H   ; 225
                        dq          056AB2779D884271DH   ; 226
                        dq          0BE83E1B0FF1525AFH   ; 227
                        dq          0FB7C65D4217E49A9H   ; 228
                        dq          06BDBE0E76D48E7D4H   ; 229
                        dq          008DF828745D9179EH   ; 230
                        dq          022EA6A9ADD53BD34H   ; 231
                        dq          0E36E141C5622200AH   ; 232
                        dq          07F805D1B8CB750EEH   ; 233
                        dq          0AFE5C7A59F58E837H   ; 234
                        dq          0E27F996A4FB1C23CH   ; 235
                        dq          0D3867DFB0775F0D0H   ; 236
                        dq          0D0E673DE6E88891AH   ; 237
                        dq          0123AEB9EAFB86C25H   ; 238
                        dq          030F1D5D5C145B895H   ; 239
                        dq          0BB434A2DEE7269E7H   ; 240
                        dq          078CB67ECF931FA38H   ; 241
                        dq          0F33B0372323BBF9CH   ; 242
                        dq          052D66336FB279C74H   ; 243
                        dq          0505F33AC0AFB4EAAH   ; 244
                        dq          0E8A5CD99A2CCE187H   ; 245
                        dq          0534974801E2D30BBH   ; 246
                        dq          08D2D5711D5876D90H   ; 247
                        dq          01F1A412891BC038EH   ; 248
                        dq          0D6E2E71D82E56648H   ; 249
                        dq          074036C3A497732B7H   ; 250
                        dq          089B67ED96361F5ABH   ; 251
                        dq          0FFED95D8F1EA02A2H   ; 252
                        dq          0E72B3BD61464D43DH   ; 253
                        dq          0A6300F170BDC4820H   ; 254
                        dq          0EBC18760ED78A77AH   ; 255
T2                      dq          0E6A6BE5A05A12138H   ; 256
                        dq          0B5A122A5B4F87C98H   ; 257
                        dq          0563C6089140B6990H   ; 258
                        dq          04C46CB2E391F5DD5H   ; 259
                        dq          0D932ADDBC9B79434H   ; 260
                        dq          008EA70E42015AFF5H   ; 261
                        dq          0D765A6673E478CF1H   ; 262
                        dq          0C4FB757EAB278D99H   ; 263
                        dq          0DF11C6862D6E0692H   ; 264
                        dq          0DDEB84F10D7F3B16H   ; 265
                        dq          06F2EF604A665EA04H   ; 266
                        dq          04A8E0F0FF0E0DFB3H   ; 267
                        dq          0A5EDEEF83DBCBA51H   ; 268
                        dq          0FC4F0A2A0EA4371EH   ; 269
                        dq          0E83E1DA85CB38429H   ; 270
                        dq          0DC8FF882BA1B1CE2H   ; 271
                        dq          0CD45505E8353E80DH   ; 272
                        dq          018D19A00D4DB0717H   ; 273
                        dq          034A0CFEDA5F38101H   ; 274
                        dq          00BE77E518887CAF2H   ; 275
                        dq          01E341438B3C45136H   ; 276
                        dq          0E05797F49089CCF9H   ; 277
                        dq          0FFD23F9DF2591D14H   ; 278
                        dq          0543DDA228595C5CDH   ; 279
                        dq          0661F81FD99052A33H   ; 280
                        dq          08736E641DB0F7B76H   ; 281
                        dq          015227725418E5307H   ; 282
                        dq          0E25F7F46162EB2FAH   ; 283
                        dq          048A8B2126C13D9FEH   ; 284
                        dq          0AFDC541792E76EEAH   ; 285
                        dq          003D912BFC6D1898FH   ; 286
                        dq          031B1AAFA1B83F51BH   ; 287
                        dq          0F1AC2796E42AB7D9H   ; 288
                        dq          040A3A7D7FCD2EBACH   ; 289
                        dq          01056136D0AFBBCC5H   ; 290
                        dq          07889E1DD9A6D0C85H   ; 291
                        dq          0D33525782A7974AAH   ; 292
                        dq          0A7E25D09078AC09BH   ; 293
                        dq          0BD4138B3EAC6EDD0H   ; 294
                        dq          0920ABFBE71EB9E70H   ; 295
                        dq          0A2A5D0F54FC2625CH   ; 296
                        dq          0C054E36B0B1290A3H   ; 297
                        dq          0F6DD59FF62FE932BH   ; 298
                        dq          03537354511A8AC7DH   ; 299
                        dq          0CA845E9172FADCD4H   ; 300
                        dq          084F82B60329D20DCH   ; 301
                        dq          079C62CE1CD672F18H   ; 302
                        dq          08B09A2ADD124642CH   ; 303
                        dq          0D0C1E96A19D9E726H   ; 304
                        dq          05A786A9B4BA9500CH   ; 305
                        dq          00E020336634C43F3H   ; 306
                        dq          0C17B474AEB66D822H   ; 307
                        dq          06A731AE3EC9BAAC2H   ; 308
                        dq          08226667AE0840258H   ; 309
                        dq          067D4567691CAECA5H   ; 310
                        dq          01D94155C4875ADB5H   ; 311
                        dq          06D00FD985B813FDFH   ; 312
                        dq          051286EFCB774CD06H   ; 313
                        dq          05E8834471FA744AFH   ; 314
                        dq          0F72CA0AEE761AE2EH   ; 315
                        dq          0BE40E4CDAEE8E09AH   ; 316
                        dq          0E9970BBB5118F665H   ; 317
                        dq          0726E4BEB33DF1964H   ; 318
                        dq          0703B000729199762H   ; 319
                        dq          04631D816F5EF30A7H   ; 320
                        dq          0B880B5B51504A6BEH   ; 321
                        dq          0641793C37ED84B6CH   ; 322
                        dq          07B21ED77F6E97D96H   ; 323
                        dq          0776306312EF96B73H   ; 324
                        dq          0AE528948E86FF3F4H   ; 325
                        dq          053DBD7F286A3F8F8H   ; 326
                        dq          016CADCE74CFC1063H   ; 327
                        dq          0005C19BDFA52C6DDH   ; 328
                        dq          068868F5D64D46AD3H   ; 329
                        dq          03A9D512CCF1E186AH   ; 330
                        dq          0367E62C2385660AEH   ; 331
                        dq          0E359E7EA77DCB1D7H   ; 332
                        dq          0526C0773749ABE6EH   ; 333
                        dq          0735AE5F9D09F734BH   ; 334
                        dq          0493FC7CC8A558BA8H   ; 335
                        dq          0B0B9C1533041AB45H   ; 336
                        dq          0321958BA470A59BDH   ; 337
                        dq          0852DB00B5F46C393H   ; 338
                        dq          091209B2BD336B0E5H   ; 339
                        dq          06E604F7D659EF19FH   ; 340
                        dq          0B99A8AE2782CCB24H   ; 341
                        dq          0CCF52AB6C814C4C7H   ; 342
                        dq          04727D9AFBE11727BH   ; 343
                        dq          07E950D0C0121B34DH   ; 344
                        dq          0756F435670AD471FH   ; 345
                        dq          0F5ADD442615A6849H   ; 346
                        dq          04E87E09980B9957AH   ; 347
                        dq          02ACFA1DF50AEE355H   ; 348
                        dq          0D898263AFD2FD556H   ; 349
                        dq          0C8F4924DD80C8FD6H   ; 350
                        dq          0CF99CA3D754A173AH   ; 351
                        dq          0FE477BACAF91BF3CH   ; 352
                        dq          0ED5371F6D690C12DH   ; 353
                        dq          0831A5C285E687094H   ; 354
                        dq          0C5D3C90A3708A0A4H   ; 355
                        dq          00F7F903717D06580H   ; 356
                        dq          019F9BB13B8FDF27FH   ; 357
                        dq          0B1BD6F1B4D502843H   ; 358
                        dq          01C761BA38FFF4012H   ; 359
                        dq          00D1530C4E2E21F3BH   ; 360
                        dq          08943CE69A7372C8AH   ; 361
                        dq          0E5184E11FEB5CE66H   ; 362
                        dq          0618BDB80BD736621H   ; 363
                        dq          07D29BAD68B574D0BH   ; 364
                        dq          081BB613E25E6FE5BH   ; 365
                        dq          0071C9C10BC07913FH   ; 366
                        dq          0C7BEEB7909AC2D97H   ; 367
                        dq          0C3E58D353BC5D757H   ; 368
                        dq          0EB017892F38F61E8H   ; 369
                        dq          0D4EFFB9C9B1CC21AH   ; 370
                        dq          099727D26F494F7ABH   ; 371
                        dq          0A3E063A2956B3E03H   ; 372
                        dq          09D4A8B9A4AA09C30H   ; 373
                        dq          03F6AB7D500090FB4H   ; 374
                        dq          09CC0F2A057268AC0H   ; 375
                        dq          03DEE9D2DEDBF42D1H   ; 376
                        dq          0330F49C87960A972H   ; 377
                        dq          0C6B2720287421B41H   ; 378
                        dq          00AC59EC07C00369CH   ; 379
                        dq          0EF4EAC49CB353425H   ; 380
                        dq          0F450244EEF0129D8H   ; 381
                        dq          08ACC46E5CAF4DEB6H   ; 382
                        dq          02FFEAB63989263F7H   ; 383
                        dq          08F7CB9FE5D7A4578H   ; 384
                        dq          05BD8F7644E634635H   ; 385
                        dq          0427A7315BF2DC900H   ; 386
                        dq          017D0C4AA2125261CH   ; 387
                        dq          03992486C93518E50H   ; 388
                        dq          0B4CBFEE0A2D7D4C3H   ; 389
                        dq          07C75D6202C5DDD8DH   ; 390
                        dq          0DBC295D8E35B6C61H   ; 391
                        dq          060B369D302032B19H   ; 392
                        dq          0CE42685FDCE44132H   ; 393
                        dq          006F3DDB9DDF65610H   ; 394
                        dq          08EA4D21DB5E148F0H   ; 395
                        dq          020B0FCE62FCD496FH   ; 396
                        dq          02C1B912358B0EE31H   ; 397
                        dq          0B28317B818F5A308H   ; 398
                        dq          0A89C1E189CA6D2CFH   ; 399
                        dq          00C6B18576AAADBC8H   ; 400
                        dq          0B65DEAA91299FAE3H   ; 401
                        dq          0FB2B794B7F1027E7H   ; 402
                        dq          004E4317F443B5BEBH   ; 403
                        dq          04B852D325939D0A6H   ; 404
                        dq          0D5AE6BEEFB207FFCH   ; 405
                        dq          0309682B281C7D374H   ; 406
                        dq          0BAE309A194C3B475H   ; 407
                        dq          08CC3F97B13B49F05H   ; 408
                        dq          098A9422FF8293967H   ; 409
                        dq          0244B16B01076FF7CH   ; 410
                        dq          0F8BF571C663D67EEH   ; 411
                        dq          01F0D6758EEE30DA1H   ; 412
                        dq          0C9B611D97ADEB9B7H   ; 413
                        dq          0B7AFD5887B6C57A2H   ; 414
                        dq          06290AE846B984FE1H   ; 415
                        dq          094DF4CDEACC1A5FDH   ; 416
                        dq          0058A5BD1C5483AFFH   ; 417
                        dq          063166CC142BA3C37H   ; 418
                        dq          08DB8526EB2F76F40H   ; 419
                        dq          0E10880036F0D6D4EH   ; 420
                        dq          09E0523C9971D311DH   ; 421
                        dq          045EC2824CC7CD691H   ; 422
                        dq          0575B8359E62382C9H   ; 423
                        dq          0FA9E400DC4889995H   ; 424
                        dq          0D1823ECB45721568H   ; 425
                        dq          0DAFD983B8206082FH   ; 426
                        dq          0AA7D29082386A8CBH   ; 427
                        dq          0269FCD4403B87588H   ; 428
                        dq          01B91F5F728BDD1E0H   ; 429
                        dq          0E4669F39040201F6H   ; 430
                        dq          07A1D7C218CF04ADEH   ; 431
                        dq          065623C29D79CE5CEH   ; 432
                        dq          02368449096C00BB1H   ; 433
                        dq          0AB9BF1879DA503BAH   ; 434
                        dq          0BC23ECB1A458058EH   ; 435
                        dq          09A58DF01BB401ECCH   ; 436
                        dq          0A070E868A85F143DH   ; 437
                        dq          04FF188307DF2239EH   ; 438
                        dq          014D565B41A641183H   ; 439
                        dq          0EE13337452701602H   ; 440
                        dq          0950E3DCF3F285E09H   ; 441
                        dq          059930254B9C80953H   ; 442
                        dq          03BF299408930DA6DH   ; 443
                        dq          0A955943F53691387H   ; 444
                        dq          0A15EDECAA9CB8784H   ; 445
                        dq          029142127352BE9A0H   ; 446
                        dq          076F0371FFF4E7AFBH   ; 447
                        dq          00239F450274F2228H   ; 448
                        dq          0BB073AF01D5E868BH   ; 449
                        dq          0BFC80571C10E96C1H   ; 450
                        dq          0D267088568222E23H   ; 451
                        dq          09671A3D48E80B5B0H   ; 452
                        dq          055B5D38AE193BB81H   ; 453
                        dq          0693AE2D0A18B04B8H   ; 454
                        dq          05C48B4ECADD5335FH   ; 455
                        dq          0FD743B194916A1CAH   ; 456
                        dq          02577018134BE98C4H   ; 457
                        dq          0E77987E83C54A4ADH   ; 458
                        dq          028E11014DA33E1B9H   ; 459
                        dq          0270CC59E226AA213H   ; 460
                        dq          071495F756D1A5F60H   ; 461
                        dq          09BE853FB60AFEF77H   ; 462
                        dq          0ADC786A7F7443DBFH   ; 463
                        dq          00904456173B29A82H   ; 464
                        dq          058BC7A66C232BD5EH   ; 465
                        dq          0F306558C673AC8B2H   ; 466
                        dq          041F639C6B6C9772AH   ; 467
                        dq          0216DEFE99FDA35DAH   ; 468
                        dq          011640CC71C7BE615H   ; 469
                        dq          093C43694565C5527H   ; 470
                        dq          0EA038E6246777839H   ; 471
                        dq          0F9ABF3CE5A3E2469H   ; 472
                        dq          0741E768D0FD312D2H   ; 473
                        dq          00144B883CED652C6H   ; 474
                        dq          0C20B5A5BA33F8552H   ; 475
                        dq          01AE69633C3435A9DH   ; 476
                        dq          097A28CA4088CFDECH   ; 477
                        dq          08824A43C1E96F420H   ; 478
                        dq          037612FA66EEEA746H   ; 479
                        dq          06B4CB165F9CF0E5AH   ; 480
                        dq          043AA1C06A0ABFB4AH   ; 481
                        dq          07F4DC26FF162796BH   ; 482
                        dq          06CBACC8E54ED9B0FH   ; 483
                        dq          0A6B7FFEFD2BB253EH   ; 484
                        dq          02E25BC95B0A29D4FH   ; 485
                        dq          086D6A58BDEF1388CH   ; 486
                        dq          0DED74AC576B6F054H   ; 487
                        dq          08030BDBC2B45805DH   ; 488
                        dq          03C81AF70E94D9289H   ; 489
                        dq          03EFF6DDA9E3100DBH   ; 490
                        dq          0B38DC39FDFCC8847H   ; 491
                        dq          0123885528D17B87EH   ; 492
                        dq          0F2DA0ED240B1B642H   ; 493
                        dq          044CEFADCD54BF9A9H   ; 494
                        dq          01312200E433C7EE6H   ; 495
                        dq          09FFCC84F3A78C748H   ; 496
                        dq          0F0CD1F72248576BBH   ; 497
                        dq          0EC6974053638CFE4H   ; 498
                        dq          02BA7B67C0CEC4E4CH   ; 499
                        dq          0AC2F4DF3E5CE32EDH   ; 500
                        dq          0CB33D14326EA4C11H   ; 501
                        dq          0A4E9044CC77E58BCH   ; 502
                        dq          05F513293D934FCEFH   ; 503
                        dq          05DC9645506E55444H   ; 504
                        dq          050DE418F317DE40AH   ; 505
                        dq          0388CB31A69DDE259H   ; 506
                        dq          02DB4A83455820A86H   ; 507
                        dq          09010A91E84711AE9H   ; 508
                        dq          04DF7F0B7B1498371H   ; 509
                        dq          0D62A2EABC0977179H   ; 510
                        dq          022FAC097AA8D5C0EH   ; 511
T3                      dq          0F49FCC2FF1DAF39BH   ; 512
                        dq          0487FD5C66FF29281H   ; 513
                        dq          0E8A30667FCDCA83FH   ; 514
                        dq          02C9B4BE3D2FCCE63H   ; 515
                        dq          0DA3FF74B93FBBBC2H   ; 516
                        dq          02FA165D2FE70BA66H   ; 517
                        dq          0A103E279970E93D4H   ; 518
                        dq          0BECDEC77B0E45E71H   ; 519
                        dq          0CFB41E723985E497H   ; 520
                        dq          0B70AAA025EF75017H   ; 521
                        dq          0D42309F03840B8E0H   ; 522
                        dq          08EFC1AD035898579H   ; 523
                        dq          096C6920BE2B2ABC5H   ; 524
                        dq          066AF4163375A9172H   ; 525
                        dq          02174ABDCCA7127FBH   ; 526
                        dq          0B33CCEA64A72FF41H   ; 527
                        dq          0F04A4933083066A5H   ; 528
                        dq          08D970ACDD7289AF5H   ; 529
                        dq          08F96E8E031C8C25EH   ; 530
                        dq          0F3FEC02276875D47H   ; 531
                        dq          0EC7BF310056190DDH   ; 532
                        dq          0F5ADB0AEBB0F1491H   ; 533
                        dq          09B50F8850FD58892H   ; 534
                        dq          04975488358B74DE8H   ; 535
                        dq          0A3354FF691531C61H   ; 536
                        dq          00702BBE481D2C6EEH   ; 537
                        dq          089FB24057DEDED98H   ; 538
                        dq          0AC3075138596E902H   ; 539
                        dq          01D2D3580172772EDH   ; 540
                        dq          0EB738FC28E6BC30DH   ; 541
                        dq          05854EF8F63044326H   ; 542
                        dq          09E5C52325ADD3BBEH   ; 543
                        dq          090AA53CF325C4623H   ; 544
                        dq          0C1D24D51349DD067H   ; 545
                        dq          02051CFEEA69EA624H   ; 546
                        dq          013220F0A862E7E4FH   ; 547
                        dq          0CE39399404E04864H   ; 548
                        dq          0D9C42CA47086FCB7H   ; 549
                        dq          0685AD2238A03E7CCH   ; 550
                        dq          0066484B2AB2FF1DBH   ; 551
                        dq          0FE9D5D70EFBF79ECH   ; 552
                        dq          05B13B9DD9C481854H   ; 553
                        dq          015F0D475ED1509ADH   ; 554
                        dq          00BEBCD060EC79851H   ; 555
                        dq          0D58C6791183AB7F8H   ; 556
                        dq          0D1187C5052F3EEE4H   ; 557
                        dq          0C95D1192E54E82FFH   ; 558
                        dq          086EEA14CB9AC6CA2H   ; 559
                        dq          03485BEB153677D5DH   ; 560
                        dq          0DD191D781F8C492AH   ; 561
                        dq          0F60866BAA784EBF9H   ; 562
                        dq          0518F643BA2D08C74H   ; 563
                        dq          08852E956E1087C22H   ; 564
                        dq          0A768CB8DC410AE8DH   ; 565
                        dq          038047726BFEC8E1AH   ; 566
                        dq          0A67738B4CD3B45AAH   ; 567
                        dq          0AD16691CEC0DDE19H   ; 568
                        dq          0C6D4319380462E07H   ; 569
                        dq          0C5A5876D0BA61938H   ; 570
                        dq          016B9FA1FA58FD840H   ; 571
                        dq          0188AB1173CA74F18H   ; 572
                        dq          0ABDA2F98C99C021FH   ; 573
                        dq          03E0580AB134AE816H   ; 574
                        dq          05F3B05B773645ABBH   ; 575
                        dq          02501A2BE5575F2F6H   ; 576
                        dq          01B2F74004E7E8BA9H   ; 577
                        dq          01CD7580371E8D953H   ; 578
                        dq          07F6ED89562764E30H   ; 579
                        dq          0B15926FF596F003DH   ; 580
                        dq          09F65293DA8C5D6B9H   ; 581
                        dq          06ECEF04DD690F84CH   ; 582
                        dq          04782275FFF33AF88H   ; 583
                        dq          0E41433083F820801H   ; 584
                        dq          0FD0DFE409A1AF9B5H   ; 585
                        dq          04325A3342CDB396BH   ; 586
                        dq          08AE77E62B301B252H   ; 587
                        dq          0C36F9E9F6655615AH   ; 588
                        dq          085455A2D92D32C09H   ; 589
                        dq          0F2C7DEA949477485H   ; 590
                        dq          063CFB4C133A39EBAH   ; 591
                        dq          083B040CC6EBC5462H   ; 592
                        dq          03B9454C8FDB326B0H   ; 593
                        dq          056F56A9E87FFD78CH   ; 594
                        dq          02DC2940D99F42BC6H   ; 595
                        dq          098F7DF096B096E2DH   ; 596
                        dq          019A6E01E3AD852BFH   ; 597
                        dq          042A99CCBDBD4B40BH   ; 598
                        dq          0A59998AF45E9C559H   ; 599
                        dq          0366295E807D93186H   ; 600
                        dq          06B48181BFAA1F773H   ; 601
                        dq          01FEC57E2157A0A1DH   ; 602
                        dq          04667446AF6201AD5H   ; 603
                        dq          0E615EBCACFB0F075H   ; 604
                        dq          0B8F31F4F68290778H   ; 605
                        dq          022713ED6CE22D11EH   ; 606
                        dq          03057C1A72EC3C93BH   ; 607
                        dq          0CB46ACC37C3F1F2FH   ; 608
                        dq          0DBB893FD02AAF50EH   ; 609
                        dq          0331FD92E600B9FCFH   ; 610
                        dq          0A498F96148EA3AD6H   ; 611
                        dq          0A8D8426E8B6A83EAH   ; 612
                        dq          0A089B274B7735CDCH   ; 613
                        dq          087F6B3731E524A11H   ; 614
                        dq          0118808E5CBC96749H   ; 615
                        dq          09906E4C7B19BD394H   ; 616
                        dq          0AFED7F7E9B24A20CH   ; 617
                        dq          06509EADEEB3644A7H   ; 618
                        dq          06C1EF1D3E8EF0EDEH   ; 619
                        dq          0B9C97D43E9798FB4H   ; 620
                        dq          0A2F2D784740C28A3H   ; 621
                        dq          07B8496476197566FH   ; 622
                        dq          07A5BE3E6B65F069DH   ; 623
                        dq          0F96330ED78BE6F10H   ; 624
                        dq          0EEE60DE77A076A15H   ; 625
                        dq          02B4BEE4AA08B9BD0H   ; 626
                        dq          06A56A63EC7B8894EH   ; 627
                        dq          002121359BA34FEF4H   ; 628
                        dq          04CBF99F8283703FCH   ; 629
                        dq          0398071350CAF30C8H   ; 630
                        dq          0D0A77A89F017687AH   ; 631
                        dq          0F1C1A9EB9E423569H   ; 632
                        dq          08C7976282DEE8199H   ; 633
                        dq          05D1737A5DD1F7ABDH   ; 634
                        dq          04F53433C09A9FA80H   ; 635
                        dq          0FA8B0C53DF7CA1D9H   ; 636
                        dq          03FD9DCBC886CCB77H   ; 637
                        dq          0C040917CA91B4720H   ; 638
                        dq          07DD00142F9D1DCDFH   ; 639
                        dq          08476FC1D4F387B58H   ; 640
                        dq          023F8E7C5F3316503H   ; 641
                        dq          0032A2244E7E37339H   ; 642
                        dq          05C87A5D750F5A74BH   ; 643
                        dq          0082B4CC43698992EH   ; 644
                        dq          0DF917BECB858F63CH   ; 645
                        dq          03270B8FC5BF86DDAH   ; 646
                        dq          010AE72BB29B5DD76H   ; 647
                        dq          0576AC94E7700362BH   ; 648
                        dq          01AD112DAC61EFB8FH   ; 649
                        dq          0691BC30EC5FAA427H   ; 650
                        dq          0FF246311CC327143H   ; 651
                        dq          03142368E30E53206H   ; 652
                        dq          071380E31E02CA396H   ; 653
                        dq          0958D5C960AAD76F1H   ; 654
                        dq          0F8D6F430C16DA536H   ; 655
                        dq          0C8FFD13F1BE7E1D2H   ; 656
                        dq          07578AE66004DDBE1H   ; 657
                        dq          005833F01067BE646H   ; 658
                        dq          0BB34B5AD3BFE586DH   ; 659
                        dq          0095F34C9A12B97F0H   ; 660
                        dq          0247AB64525D60CA8H   ; 661
                        dq          0DCDBC6F3017477D1H   ; 662
                        dq          04A2E14D4DECAD24DH   ; 663
                        dq          0BDB5E6D9BE0A1EEBH   ; 664
                        dq          02A7E70F7794301ABH   ; 665
                        dq          0DEF42D8A270540FDH   ; 666
                        dq          001078EC0A34C22C1H   ; 667
                        dq          0E5DE511AF4C16387H   ; 668
                        dq          07EBB3A52BD9A330AH   ; 669
                        dq          077697857AA7D6435H   ; 670
                        dq          0004E831603AE4C32H   ; 671
                        dq          0E7A21020AD78E312H   ; 672
                        dq          09D41A70C6AB420F2H   ; 673
                        dq          028E06C18EA1141E6H   ; 674
                        dq          0D2B28CBD984F6B28H   ; 675
                        dq          026B75F6C446E9D83H   ; 676
                        dq          0BA47568C4D418D7FH   ; 677
                        dq          0D80BADBFE6183D8EH   ; 678
                        dq          00E206D7F5F166044H   ; 679
                        dq          0E258A43911CBCA3EH   ; 680
                        dq          0723A1746B21DC0BCH   ; 681
                        dq          0C7CAA854F5D7CDD3H   ; 682
                        dq          07CAC32883D261D9CH   ; 683
                        dq          07690C26423BA942CH   ; 684
                        dq          017E55524478042B8H   ; 685
                        dq          0E0BE477656A2389FH   ; 686
                        dq          04D289B5E67AB2DA0H   ; 687
                        dq          044862B9C8FBBFD31H   ; 688
                        dq          0B47CC8049D141365H   ; 689
                        dq          0822C1B362B91C793H   ; 690
                        dq          04EB14655FB13DFD8H   ; 691
                        dq          01ECBBA0714E2A97BH   ; 692
                        dq          06143459D5CDE5F14H   ; 693
                        dq          053A8FBF1D5F0AC89H   ; 694
                        dq          097EA04D81C5E5B00H   ; 695
                        dq          0622181A8D4FDB3F3H   ; 696
                        dq          0E9BCD341572A1208H   ; 697
                        dq          01411258643CCE58AH   ; 698
                        dq          09144C5FEA4C6E0A4H   ; 699
                        dq          00D33D06565CF620FH   ; 700
                        dq          054A48D489F219CA1H   ; 701
                        dq          0C43E5EAC6D63C821H   ; 702
                        dq          0A9728B3A72770DAFH   ; 703
                        dq          0D7934E7B20DF87EFH   ; 704
                        dq          0E35503B61A3E86E5H   ; 705
                        dq          0CAE321FBC819D504H   ; 706
                        dq          0129A50B3AC60BFA6H   ; 707
                        dq          0CD5E68EA7E9FB6C3H   ; 708
                        dq          0B01C90199483B1C7H   ; 709
                        dq          03DE93CD5C295376CH   ; 710
                        dq          0AED52EDF2AB9AD13H   ; 711
                        dq          02E60F512C0A07884H   ; 712
                        dq          0BC3D86A3E36210C9H   ; 713
                        dq          035269D9B163951CEH   ; 714
                        dq          00C7D6E2AD0CDB5FAH   ; 715
                        dq          059E86297D87F5733H   ; 716
                        dq          0298EF221898DB0E7H   ; 717
                        dq          055000029D1A5AA7EH   ; 718
                        dq          08BC08AE1B5061B45H   ; 719
                        dq          0C2C31C2B6C92703AH   ; 720
                        dq          094CC596BAF25EF42H   ; 721
                        dq          00A1D73DB22540456H   ; 722
                        dq          004B6A0F9D9C4179AH   ; 723
                        dq          0EFFDAFA2AE3D3C60H   ; 724
                        dq          0F7C8075BB49496C4H   ; 725
                        dq          09CC5C7141D1CD4E3H   ; 726
                        dq          078BD1638218E5534H   ; 727
                        dq          0B2F11568F850246AH   ; 728
                        dq          0EDFABCFA9502BC29H   ; 729
                        dq          0796CE5F2DA23051BH   ; 730
                        dq          0AAE128B0DC93537CH   ; 731
                        dq          03A493DA0EE4B29AEH   ; 732
                        dq          0B5DF6B2C416895D7H   ; 733
                        dq          0FCABBD25122D7F37H   ; 734
                        dq          070810B58105DC4B1H   ; 735
                        dq          0E10FDD37F7882A90H   ; 736
                        dq          0524DCAB5518A3F5CH   ; 737
                        dq          03C9E85878451255BH   ; 738
                        dq          04029828119BD34E2H   ; 739
                        dq          074A05B6F5D3CECCBH   ; 740
                        dq          0B610021542E13ECAH   ; 741
                        dq          00FF979D12F59E2ACH   ; 742
                        dq          06037DA27E4F9CC50H   ; 743
                        dq          05E92975A0DF1847DH   ; 744
                        dq          0D66DE190D3E623FEH   ; 745
                        dq          05032D6B87B568048H   ; 746
                        dq          09A36B7CE8235216EH   ; 747
                        dq          080272A7A24F64B4AH   ; 748
                        dq          093EFED8B8C6916F7H   ; 749
                        dq          037DDBFF44CCE1555H   ; 750
                        dq          04B95DB5D4B99BD25H   ; 751
                        dq          092D3FDA169812FC0H   ; 752
                        dq          0FB1A4A9A90660BB6H   ; 753
                        dq          0730C196946A4B9B2H   ; 754
                        dq          081E289AA7F49DA68H   ; 755
                        dq          064669A0F83B1A05FH   ; 756
                        dq          027B3FF7D9644F48BH   ; 757
                        dq          0CC6B615C8DB675B3H   ; 758
                        dq          0674F20B9BCEBBE95H   ; 759
                        dq          06F31238275655982H   ; 760
                        dq          05AE488713E45CF05H   ; 761
                        dq          0BF619F9954C21157H   ; 762
                        dq          0EABAC46040A8EAE9H   ; 763
                        dq          0454C6FE9F2C0C1CDH   ; 764
                        dq          0419CF6496412691CH   ; 765
                        dq          0D3DC3BEF265B0F70H   ; 766
                        dq          06D0E60F5C3578A9EH   ; 767
T4                      dq          05B0E608526323C55H   ; 768
                        dq          01A46C1A9FA1B59F5H   ; 769
                        dq          0A9E245A17C4C8FFAH   ; 770
                        dq          065CA5159DB2955D7H   ; 771
                        dq          005DB0A76CE35AFC2H   ; 772
                        dq          081EAC77EA9113D45H   ; 773
                        dq          0528EF88AB6AC0A0DH   ; 774
                        dq          0A09EA253597BE3FFH   ; 775
                        dq          0430DDFB3AC48CD56H   ; 776
                        dq          0C4B3A67AF45CE46FH   ; 777
                        dq          04ECECFD8FBE2D05EH   ; 778
                        dq          03EF56F10B39935F0H   ; 779
                        dq          00B22D6829CD619C6H   ; 780
                        dq          017FD460A74DF2069H   ; 781
                        dq          06CF8CC8E8510ED40H   ; 782
                        dq          0D6C824BF3A6ECAA7H   ; 783
                        dq          061243D581A817049H   ; 784
                        dq          0048BACB6BBC163A2H   ; 785
                        dq          0D9A38AC27D44CC32H   ; 786
                        dq          07FDDFF5BAAF410ABH   ; 787
                        dq          0AD6D495AA804824BH   ; 788
                        dq          0E1A6A74F2D8C9F94H   ; 789
                        dq          0D4F7851235DEE8E3H   ; 790
                        dq          0FD4B7F886540D893H   ; 791
                        dq          0247C20042AA4BFDAH   ; 792
                        dq          0096EA1C517D1327CH   ; 793
                        dq          0D56966B4361A6685H   ; 794
                        dq          0277DA5C31221057DH   ; 795
                        dq          094D59893A43ACFF7H   ; 796
                        dq          064F0C51CCDC02281H   ; 797
                        dq          03D33BCC4FF6189DBH   ; 798
                        dq          0E005CB184CE66AF1H   ; 799
                        dq          0FF5CCD1D1DB99BEAH   ; 800
                        dq          0B0B854A7FE42980FH   ; 801
                        dq          07BD46A6A718D4B9FH   ; 802
                        dq          0D10FA8CC22A5FD8CH   ; 803
                        dq          0D31484952BE4BD31H   ; 804
                        dq          0C7FA975FCB243847H   ; 805
                        dq          04886ED1E5846C407H   ; 806
                        dq          028CDDB791EB70B04H   ; 807
                        dq          0C2B00BE2F573417FH   ; 808
                        dq          05C9590452180F877H   ; 809
                        dq          07A6BDDFFF370EB00H   ; 810
                        dq          0CE509E38D6D9D6A4H   ; 811
                        dq          0EBEB0F00647FA702H   ; 812
                        dq          01DCC06CF76606F06H   ; 813
                        dq          0E4D9F28BA286FF0AH   ; 814
                        dq          0D85A305DC918C262H   ; 815
                        dq          0475B1D8732225F54H   ; 816
                        dq          02D4FB51668CCB5FEH   ; 817
                        dq          0A679B9D9D72BBA20H   ; 818
                        dq          053841C0D912D43A5H   ; 819
                        dq          03B7EAA48BF12A4E8H   ; 820
                        dq          0781E0E47F22F1DDFH   ; 821
                        dq          0EFF20CE60AB50973H   ; 822
                        dq          020D261D19DFFB742H   ; 823
                        dq          016A12B03062A2E39H   ; 824
                        dq          01960EB2239650495H   ; 825
                        dq          0251C16FED50EB8B8H   ; 826
                        dq          09AC0C330F826016EH   ; 827
                        dq          0ED152665953E7671H   ; 828
                        dq          002D63194A6369570H   ; 829
                        dq          05074F08394B1C987H   ; 830
                        dq          070BA598C90B25CE1H   ; 831
                        dq          0794A15810B9742F6H   ; 832
                        dq          00D5925E9FCAF8C6CH   ; 833
                        dq          03067716CD868744EH   ; 834
                        dq          0910AB077E8D7731BH   ; 835
                        dq          06A61BBDB5AC42F61H   ; 836
                        dq          093513EFBF0851567H   ; 837
                        dq          0F494724B9E83E9D5H   ; 838
                        dq          0E887E1985C09648DH   ; 839
                        dq          034B1D3C675370CFDH   ; 840
                        dq          0DC35E433BC0D255DH   ; 841
                        dq          0D0AAB84234131BE0H   ; 842
                        dq          008042A50B48B7EAFH   ; 843
                        dq          09997C4EE44A3AB35H   ; 844
                        dq          0829A7B49201799D0H   ; 845
                        dq          0263B8307B7C54441H   ; 846
                        dq          0752F95F4FD6A6CA6H   ; 847
                        dq          0927217402C08C6E5H   ; 848
                        dq          02A8AB754A795D9EEH   ; 849
                        dq          0A442F7552F72943DH   ; 850
                        dq          02C31334E19781208H   ; 851
                        dq          04FA98D7CEAEE6291H   ; 852
                        dq          055C3862F665DB309H   ; 853
                        dq          0BD0610175D53B1F3H   ; 854
                        dq          046FE6CB840413F27H   ; 855
                        dq          03FE03792DF0CFA59H   ; 856
                        dq          0CFE700372EB85E8FH   ; 857
                        dq          0A7BE29E7ADBCE118H   ; 858
                        dq          0E544EE5CDE8431DDH   ; 859
                        dq          08A781B1B41F1873EH   ; 860
                        dq          0A5C94C78A0D2F0E7H   ; 861
                        dq          039412E2877B60728H   ; 862
                        dq          0A1265EF3AFC9A62CH   ; 863
                        dq          0BCC2770C6A2506C5H   ; 864
                        dq          03AB66DD5DCE1CE12H   ; 865
                        dq          0E65499D04A675B37H   ; 866
                        dq          07D8F523481BFD216H   ; 867
                        dq          00F6F64FCEC15F389H   ; 868
                        dq          074EFBE618B5B13C8H   ; 869
                        dq          0ACDC82B714273E1DH   ; 870
                        dq          0DD40BFE003199D17H   ; 871
                        dq          037E99257E7E061F8H   ; 872
                        dq          0FA52626904775AAAH   ; 873
                        dq          08BBBF63A463D56F9H   ; 874
                        dq          0F0013F1543A26E64H   ; 875
                        dq          0A8307E9F879EC898H   ; 876
                        dq          0CC4C27A4150177CCH   ; 877
                        dq          01B432F2CCA1D3348H   ; 878
                        dq          0DE1D1F8F9F6FA013H   ; 879
                        dq          0606602A047A7DDD6H   ; 880
                        dq          0D237AB64CC1CB2C7H   ; 881
                        dq          09B938E7225FCD1D3H   ; 882
                        dq          0EC4E03708E0FF476H   ; 883
                        dq          0FEB2FBDA3D03C12DH   ; 884
                        dq          0AE0BCED2EE43889AH   ; 885
                        dq          022CB8923EBFB4F43H   ; 886
                        dq          069360D013CF7396DH   ; 887
                        dq          0855E3602D2D4E022H   ; 888
                        dq          0073805BAD01F784CH   ; 889
                        dq          033E17A133852F546H   ; 890
                        dq          0DF4874058AC7B638H   ; 891
                        dq          0BA92B29C678AA14AH   ; 892
                        dq          00CE89FC76CFAADCDH   ; 893
                        dq          05F9D4E0908339E34H   ; 894
                        dq          0F1AFE9291F5923B9H   ; 895
                        dq          06E3480F60F4A265FH   ; 896
                        dq          0EEBF3A2AB29B841CH   ; 897
                        dq          0E21938A88F91B4ADH   ; 898
                        dq          057DFEFF845C6D3C3H   ; 899
                        dq          02F006B0BF62CAAF2H   ; 900
                        dq          062F479EF6F75EE78H   ; 901
                        dq          011A55AD41C8916A9H   ; 902
                        dq          0F229D29084FED453H   ; 903
                        dq          042F1C27B16B000E6H   ; 904
                        dq          02B1F76749823C074H   ; 905
                        dq          04B76ECA3C2745360H   ; 906
                        dq          08C98F463B91691BDH   ; 907
                        dq          014BCC93CF1ADE66AH   ; 908
                        dq          08885213E6D458397H   ; 909
                        dq          08E177DF0274D4711H   ; 910
                        dq          0B49B73B5503F2951H   ; 911
                        dq          010168168C3F96B6BH   ; 912
                        dq          00E3D963B63CAB0AEH   ; 913
                        dq          08DFC4B5655A1DB14H   ; 914
                        dq          0F789F1356E14DE5CH   ; 915
                        dq          0683E68AF4E51DAC1H   ; 916
                        dq          0C9A84F9D8D4B0FD9H   ; 917
                        dq          03691E03F52A0F9D1H   ; 918
                        dq          05ED86E46E1878E80H   ; 919
                        dq          03C711A0E99D07150H   ; 920
                        dq          05A0865B20C4E9310H   ; 921
                        dq          056FBFC1FE4F0682EH   ; 922
                        dq          0EA8D5DE3105EDF9BH   ; 923
                        dq          071ABFDB12379187AH   ; 924
                        dq          02EB99DE1BEE77B9CH   ; 925
                        dq          021ECC0EA33CF4523H   ; 926
                        dq          059A4D7521805C7A1H   ; 927
                        dq          03896F5EB56AE7C72H   ; 928
                        dq          0AA638F3DB18F75DCH   ; 929
                        dq          09F39358DABE9808EH   ; 930
                        dq          0B7DEFA91C00B72ACH   ; 931
                        dq          06B5541FD62492D92H   ; 932
                        dq          06DC6DEE8F92E4D5BH   ; 933
                        dq          0353F57ABC4BEEA7EH   ; 934
                        dq          0735769D6DA5690CEH   ; 935
                        dq          00A234AA642391484H   ; 936
                        dq          0F6F9508028F80D9DH   ; 937
                        dq          0B8E319A27AB3F215H   ; 938
                        dq          031AD9C1151341A4DH   ; 939
                        dq          0773C22A57BEF5805H   ; 940
                        dq          045C7561A07968633H   ; 941
                        dq          0F913DA9E249DBE36H   ; 942
                        dq          0DA652D9B78A64C68H   ; 943
                        dq          04C27A97F3BC334EFH   ; 944
                        dq          076621220E66B17F4H   ; 945
                        dq          0967743899ACD7D0BH   ; 946
                        dq          0F3EE5BCAE0ED6782H   ; 947
                        dq          0409F753600C879FCH   ; 948
                        dq          006D09A39B5926DB6H   ; 949
                        dq          06F83AEB0317AC588H   ; 950
                        dq          001E6CA4A86381F21H   ; 951
                        dq          066FF3462D19F3025H   ; 952
                        dq          072207C24DDFD3BFBH   ; 953
                        dq          04AF6B6D3E2ECE2EBH   ; 954
                        dq          09C994DBEC7EA08DEH   ; 955
                        dq          049ACE597B09A8BC4H   ; 956
                        dq          0B38C4766CF0797BAH   ; 957
                        dq          0131B9373C57C2A75H   ; 958
                        dq          0B1822CCE61931E58H   ; 959
                        dq          09D7555B909BA1C0CH   ; 960
                        dq          0127FAFDD937D11D2H   ; 961
                        dq          029DA3BADC66D92E4H   ; 962
                        dq          0A2C1D57154C2ECBCH   ; 963
                        dq          058C5134D82F6FE24H   ; 964
                        dq          01C3AE3515B62274FH   ; 965
                        dq          0E907C82E01CB8126H   ; 966
                        dq          0F8ED091913E37FCBH   ; 967
                        dq          03249D8F9C80046C9H   ; 968
                        dq          080CF9BEDE388FB63H   ; 969
                        dq          01881539A116CF19EH   ; 970
                        dq          05103F3F76BD52457H   ; 971
                        dq          015B7E6F5AE47F7A8H   ; 972
                        dq          0DBD7C6DED47E9CCFH   ; 973
                        dq          044E55C410228BB1AH   ; 974
                        dq          0B647D4255EDB4E99H   ; 975
                        dq          05D11882BB8AAFC30H   ; 976
                        dq          0F5098BBB29D3212AH   ; 977
                        dq          08FB5EA14E90296B3H   ; 978
                        dq          0677B942157DD025AH   ; 979
                        dq          0FB58E7C0A390ACB5H   ; 980
                        dq          089D3674C83BD4A01H   ; 981
                        dq          09E2DA4DF4BF3B93BH   ; 982
                        dq          0FCC41E328CAB4829H   ; 983
                        dq          003F38C96BA582C52H   ; 984
                        dq          0CAD1BDBD7FD85DB2H   ; 985
                        dq          0BBB442C16082AE83H   ; 986
                        dq          0B95FE86BA5DA9AB0H   ; 987
                        dq          0B22E04673771A93FH   ; 988
                        dq          0845358C9493152D8H   ; 989
                        dq          0BE2A488697B4541EH   ; 990
                        dq          095A2DC2DD38E6966H   ; 991
                        dq          0C02C11AC923C852BH   ; 992
                        dq          02388B1990DF2A87BH   ; 993
                        dq          07C8008FA1B4F37BEH   ; 994
                        dq          01F70D0C84D54E503H   ; 995
                        dq          05490ADEC7ECE57D4H   ; 996
                        dq          0002B3C27D9063A3AH   ; 997
                        dq          07EAEA3848030A2BFH   ; 998
                        dq          0C602326DED2003C0H   ; 999
                        dq          083A7287D69A94086H   ; 1000
                        dq          0C57A5FCB30F57A8AH   ; 1001
                        dq          0B56844E479EBE779H   ; 1002
                        dq          0A373B40F05DCBCE9H   ; 1003
                        dq          0D71A786E88570EE2H   ; 1004
                        dq          0879CBACDBDE8F6A0H   ; 1005
                        dq          0976AD1BCC164A32FH   ; 1006
                        dq          0AB21E25E9666D78BH   ; 1007
                        dq          0901063AAE5E5C33CH   ; 1008
                        dq          09818B34448698D90H   ; 1009
                        dq          0E36487AE3E1E8ABBH   ; 1010
                        dq          0AFBDF931893BDCB4H   ; 1011
                        dq          06345A0DC5FBBD519H   ; 1012
                        dq          08628FE269B9465CAH   ; 1013
                        dq          01E5D01603F9C51ECH   ; 1014
                        dq          04DE44006A15049B7H   ; 1015
                        dq          0BF6C70E5F776CBB1H   ; 1016
                        dq          0411218F2EF552BEDH   ; 1017
                        dq          0CB0C0708705A36A3H   ; 1018
                        dq          0E74D14754F986044H   ; 1019
                        dq          0CD56D9430EA8280EH   ; 1020
                        dq          0C12591D7535F5065H   ; 1021
                        dq          0C83223F1720AEF96H   ; 1022
                        dq          0C3A0396F7363A51FH   ; 1023

                        ALIGN       16                              ; need alignment for sse2
const_A5A5A5A5A5A5A5A5  dq          0A5A5A5A5A5A5A5A5H
                        ALIGN       16
const_0123456789ABCDEF  dq          00123456789ABCDEFH
                        ALIGN       16
const_FFFFFFFFFFFFFFFF  dq          0FFFFFFFFFFFFFFFFH

                        .code

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; start of pure p5 code
; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

a_l                     textequ     <eax>
a_h                     textequ     <edx>
b_l                     textequ     <ebx>
b_h                     textequ     <ecx>
c_l                     textequ     <esi>
c_h                     textequ     <edi>
reg_temp                textequ     <ebp>

LOAD64                  MACRO       reg1_h:REQ, reg1_l:REQ, reg2_h:REQ, reg2_l:REQ      ; mov   reg1_h:reg1_l, reg2_h:reg2_l
                        mov         reg1_l, reg2_l
                        mov         reg1_h, reg2_h
                        ENDM
LOAD64RM                MACRO       reg_h:REQ, reg_l:REQ, mem:REQ                       ; mov   reg_h:reg_l, mem
                        LOAD64      reg_h, reg_l, dword ptr [mem+4], dword ptr [mem]
                        ENDM
LOAD64MR                MACRO       mem:REQ, reg_h:REQ, reg_l:REQ                       ; mov   mem, reg_h:reg_l
                        LOAD64      dword ptr [mem+4], dword ptr [mem], reg_h, reg_l
                        ENDM

ADD64                   MACRO       reg1_h:REQ, reg1_l:REQ, reg2_h:REQ, reg2_l:REQ      ; add   reg1_h:reg1_l, reg2_h:reg2_l
                        add         reg1_l, reg2_l
                        adc         reg1_h, reg2_h
                        ENDM
ADD64RM                 MACRO       reg_h:REQ, reg_l:REQ, mem:REQ                       ; add   reg_h:reg_l, mem
                        ADD64       reg_h, reg_l, dword ptr [mem+4], dword ptr [mem]
                        ENDM
ADD64MR                 MACRO       mem:REQ, reg_h:REQ, reg_l:REQ                       ; add   mem, reg_h:reg_l
                        ADD64       dword ptr [mem+4], dword ptr [mem], reg_h, reg_l
                        ENDM

SUB64                   MACRO       reg1_h:REQ, reg1_l:REQ, reg2_h:REQ, reg2_l:REQ      ; sub   reg1_h:reg1_l, reg2_h:reg2_l
                        sub         reg1_l, reg2_l
                        sbb         reg1_h, reg2_h
                        ENDM
SUB64RM                 MACRO       reg_h:REQ, reg_l:REQ, mem:REQ                       ; sub   reg_h:reg_l, mem
                        SUB64       reg_h, reg_l, dword ptr [mem+4], dword ptr [mem]
                        ENDM
SUB64MR                 MACRO       mem:REQ, reg_h:REQ, reg_l:REQ                       ; sub   mem, reg_h:reg_l
                        SUB64       dword ptr [mem+4], dword ptr [mem], reg_h, reg_l
                        ENDM

XOR64                   MACRO       reg1_h:REQ, reg1_l:REQ, reg2_h:REQ, reg2_l:REQ      ; xor   reg1_h:reg1_l, reg2_h:reg2_l
                        xor         reg1_l, reg2_l
                        xor         reg1_h, reg2_h
                        ENDM
XOR64RM                 MACRO       reg_h:REQ, reg_l:REQ, mem:REQ                       ; xor   reg_h:reg_l, mem
                        XOR64       reg_h, reg_l, dword ptr [mem+4], dword ptr [mem]
                        ENDM
XOR64MR                 MACRO       mem:REQ, reg_h:REQ, reg_l:REQ                       ; xor   mem, reg_h:reg_l
                        XOR64       dword ptr [mem+4], dword ptr [mem], reg_h, reg_l
                        ENDM

SHR64                   MACRO       reg_h:REQ, reg_l:REQ, s:REQ                         ; shr   reg_h:reg_l, s
                        shrd        reg_l, reg_h, s
                        shr         reg_h, s
                        ENDM

SHL64                   MACRO       reg_h:REQ, reg_l:REQ, s:REQ                         ; shl   reg_h:reg_l, s
                        IF          s eq 2
                        add         reg_l, reg_l
                        adc         reg_h, reg_h
                        add         reg_l, reg_l                                        ; high latencies for SHL on P4...
                        adc         reg_h, reg_h                                        ; using add for s=3 increases
                        ELSE                                                            ; execution time
                        shld        reg_h, reg_l, s
                        shl         reg_l, s
                        ENDIF
                        ENDM

NOT64                   MACRO       reg_h:REQ, reg_l:REQ                                ; not   reg_h:reg_l
                        not         reg_l
                        not         reg_h
                        ENDM

LOADX                   MACRO       reg_h:REQ, reg_l:REQ, count                         ; mov   reg_h:reg_l, x[count]
                        LOAD64RM    reg_h, reg_l, _x+count*8
                        ENDM

STOREX                  MACRO       reg_h:REQ, reg_l:REQ, count                         ; mov   x[count], reg_h:reg_l
                        LOAD64MR    _x+count*8, reg_h, reg_l
                        ENDM

XORX                    MACRO       reg_h:REQ, reg_l:REQ, count                         ; xor   reg_h:reg_l, x[count]
                        XOR64RM     reg_h, reg_l, _x+count*8
                        ENDM


TIGERRND                MACRO       count:REQ,mulval:REQ
; c = c ^ _x[count]
; a = a - (T1[( c_l    &0ffh)*8]^T2[((c_l>>16)&0ffh)*8)^T3[( c_h    &0ffh)*8]^T4[((c_h>>16)&0ffh)*8]
; b = b + (T4[((c_l>>8)&0ffh)*8]^T3[((c_l>>24)&0ffh)*8)^T2[((c_h>>8)&0ffh)*8]^T1[((c_h>>24)&0ffh)*8]
; b = b * mul
                        XORX        c_h, c_l, count

                        LOAD64MR    _t, b_h, b_l                                        ; we need another 64-bit register for temp storage

                        mov         reg_temp, c_l
                        and         reg_temp, 0ffh
                        LOAD64RM    b_h, b_l, T1+reg_temp*8
                        mov         reg_temp, c_l
                        and         reg_temp, 0ff0000h
                        shr         reg_temp, 16
                        XOR64RM     b_h, b_l, T2+reg_temp*8
                        mov         reg_temp, c_h
                        and         reg_temp, 0ffh
                        XOR64RM     b_h, b_l, T3+reg_temp*8
                        mov         reg_temp, c_h
                        and         reg_temp, 0ff0000h
                        shr         reg_temp, 16
                        XOR64RM     b_h, b_l, T4+reg_temp*8
                        SUB64       a_h, a_l, b_h, b_l

                        LOAD64RM    b_h, b_l, _t
                        LOAD64MR    _t, a_h, a_l

                        mov         reg_temp, c_l
                        and         reg_temp, 0ff00h
                        shr         reg_temp, 8
                        LOAD64RM    a_h, a_l, T4+reg_temp*8
                        mov         reg_temp, c_l
                        and         reg_temp, 0ff000000h
                        shr         reg_temp, 24
                        XOR64RM     a_h, a_l, T3+reg_temp*8
                        mov         reg_temp, c_h
                        and         reg_temp, 0ff00h
                        shr         reg_temp, 8
                        XOR64RM     a_h, a_l, T2+reg_temp*8
                        mov         reg_temp, c_h
                        and         reg_temp, 0ff000000h
                        shr         reg_temp, 24
                        XOR64RM     a_h, a_l, T1+reg_temp*8
                        ADD64       b_h, b_l, a_h, a_l

                        LOAD64      a_h, a_l, b_h, b_l
                        IF          mulval eq 5
                        SHL64       b_h, b_l, 2
                        ADD64       b_h, b_l, a_h, a_l
                        ELSEIF      mulval eq 7
                        SHL64       b_h, b_l, 3
                        SUB64       b_h, b_l, a_h, a_l
                        ELSEIF      mulval eq 9
                        SHL64       b_h, b_l, 3
                        ADD64       b_h, b_l, a_h, a_l
                        ELSE
                        .ERR        'invalid mul value in TIGERRND'
                        ENDIF

                        LOAD64RM    a_h, a_l, _t

                        ENDM

ROTATEVARS              MACRO
;tempa=a;a=b;b=c;c=tempa
reg_t                   textequ     a_l
a_l                     textequ     b_l
b_l                     textequ     c_l
c_l                     textequ     reg_t
reg_t                   textequ     a_h
a_h                     textequ     b_h
b_h                     textequ     c_h
c_h                     textequ     reg_t
                        ENDM

TIGERROUND              MACRO       mulval:REQ

count                   =           0
                        REPEAT      8
                        TIGERRND    count, mulval
count                   =           count + 1
                        ROTATEVARS
                        ENDM
                        ENDM

TIGERSAVEVARS           MACRO
                        LOAD64MR    _aa, a_h, a_l
                        LOAD64MR    _bb, b_h, b_l
                        LOAD64MR    _cc, c_h, c_l
                        ENDM

TIGERRESTOREVARS        MACRO
                        LOAD64RM    a_h, a_l, _aa
                        LOAD64RM    b_h, b_l, _bb
                        LOAD64RM    c_h, c_l, _cc
                        ENDM

TIGERKEYSCHEDULE        MACRO

                        TIGERSAVEVARS

; x[0] = x[0] - x[7]^0A5A5A5A5A5A5A5A5H
                        LOADX       c_h, c_l, 7
                        XOR64       c_h, c_l, 0A5A5A5A5H, 0A5A5A5A5H
                        LOADX       a_h, a_l, 0
                        SUB64       a_h, a_l, c_h, c_l
                        STOREX      a_h, a_l, 0

; x[1] = x[1] ^ x[0]
                        LOADX       b_h, b_l, 1
                        XOR64       b_h, b_l, a_h, a_l
                        STOREX      b_h, b_l, 1

; x[2] = x[2] + x[1]
                        LOADX       c_h, c_l, 2
                        ADD64       c_h, c_l, b_h, b_l
                        STOREX      c_h, c_l, 2

; x[3] = x[3] - x[2]^(~x[1]<<19)
                        NOT64       b_h, b_l
                        SHL64       b_h, b_l, 19
                        XOR64       c_h, c_l, b_h, b_l
                        LOADX       a_h, a_l, 3
                        SUB64       a_h, a_l, c_h, c_l
                        STOREX      a_h, a_l, 3

; x[4] = x[4] ^ x[3]
                        LOADX       b_h, b_l, 4
                        XOR64       b_h, b_l, a_h, a_l
                        STOREX      b_h, b_l, 4
                        
; x[5] = x[5] + x[4]
                        LOADX       c_h, c_l, 5
                        ADD64       c_h, c_l, b_h, b_l
                        STOREX      c_h, c_l, 5

; x[6] = x[6] - x[5]^(~x[4]>>23)
                        NOT64       b_h, b_l
                        SHR64       b_h, b_l, 23
                        XOR64       c_h, c_l, b_h, b_l
                        LOADX       a_h, a_l, 6
                        SUB64       a_h, a_l, c_h, c_l
                        STOREX      a_h, a_l, 6

; x[7] = x[7] ^ x[6]
                        LOADX       b_h, b_l, 7
                        XOR64       b_h, b_l, a_h, a_l
                        STOREX      b_h, b_l, 7

; x[0] = x[0] + x[7]
                        LOADX       c_h, c_l, 0
                        ADD64       c_h, c_l, b_h, b_l
                        STOREX      c_h, c_l, 0

; x[1] = x[1] - x[0]^(~x[7]<<19)
                        NOT64       b_h, b_l
                        SHL64       b_h, b_l, 19
                        XOR64       c_h, c_l, b_h, b_l
                        LOADX       a_h, a_l, 1
                        SUB64       a_h, a_l, c_h, c_l
                        STOREX      a_h, a_l, 1
                        
; x[2] = x[2] ^ x[1]
                        LOADX       b_h, b_l, 2
                        XOR64       b_h, b_l, a_h, a_l
                        STOREX      b_h, b_l, 2

; x[3] = x[3] + x[2]
                        LOADX       c_h, c_l, 3
                        ADD64       c_h, c_l, b_h, b_l
                        STOREX      c_h, c_l, 3

; x[4] = x[4] - x[3]^(~x[2]>>23)
                        NOT64       b_h, b_l
                        SHR64       b_h, b_l, 23
                        XOR64       c_h, c_l, b_h, b_l
                        LOADX       a_h, a_l, 4
                        SUB64       a_h, a_l, c_h, c_l
                        STOREX      a_h, a_l, 4

; x[5] = x[5] ^ x[4]
                        LOADX       b_h, b_l, 5
                        XOR64       b_h, b_l, a_h, a_l
                        STOREX      b_h, b_l, 5

; x[6] = x[6] + x[5]
                        LOADX       c_h, c_l, 6
                        ADD64       c_h, c_l, b_h, b_l
                        STOREX      c_h, c_l, 6

; x[7] = x[7] - x[6]^0123456789ABCDEFH
                        XOR64       c_h, c_l, 01234567H, 89ABCDEFH
                        SUB64MR     _x+7*8, c_h, c_l

                        TIGERRESTOREVARS
                        
                        ENDM

                        ALIGN       16

TigerTree_Tiger_p5      PROC        PUBLIC  _Data:DWORD, _State:DWORD
; Compiles a Block of 8 64-bit words that can be found at _Data
                        pusha

__Data                  textequ     <[esp+36+96]>
__State                 textequ     <[esp+40+96]>                        
_x                      textequ     <esp+32>
_t                      textequ     <esp+24>
_aa                     textequ     <esp+16>
_bb                     textequ     <esp+8>
_cc                     textequ     <esp>
                        sub         esp, 96
                        mov         reg_temp, __Data
count                   =           0
                        REPEAT      8
                        LOAD64RM    a_h, a_l, reg_temp+count*8
                        STOREX      a_h, a_l, count
count                   =           count + 1
                        ENDM

                        mov         reg_temp, __State
                        LOAD64RM    a_h, a_l, reg_temp+m_nState0
                        LOAD64RM    b_h, b_l, reg_temp+m_nState1
                        LOAD64RM    c_h, c_l, reg_temp+m_nState2

                        TIGERROUND  5
                        TIGERKEYSCHEDULE
                        TIGERROUND  7
                        TIGERKEYSCHEDULE
                        TIGERROUND  9

                        mov         reg_temp, __State
                        XOR64MR     reg_temp+m_nState0, a_h, a_l
                        SUB64RM     b_h, b_l, reg_temp+m_nState1
                        LOAD64MR    reg_temp+m_nState1, b_h, b_l
                        ADD64MR     reg_temp+m_nState2, c_h, c_l

                        add         esp, 96
                        popa
                        ret 8

TigerTree_Tiger_p5      ENDP

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; end of pure p5 code
; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; start of SSE2 code
; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

                        .mmx
                        .xmm

reg_temp1               textequ     <eax>
reg_temp2               textequ     <ebx>
reg_temp3               textequ     <ecx>
reg_temp4               textequ     <edx>
reg_temp5               textequ     <esi>

mmx_a                   textequ     <mm0>
mmx_b                   textequ     <mm1>
mmx_c                   textequ     <mm2>
mmx_temp1               textequ     <mm3>
mmx_temp2               textequ     <mm4>
mmx_temp3               textequ     <mm5>
mmx_temp4               textequ     <mm6>
mmx_temp5               textequ     <mm7>

XORXSSE2                MACRO       reg:REQ, count:REQ              ; xor reg, x[count]
                        IF          count eq 0
                        movdq2q     mmx_temp3, xmm0
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 1
                        movdq2q     mmx_temp3, xmm1
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 2
                        movdq2q     mmx_temp3, xmm2
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 3
                        movdq2q     mmx_temp3, xmm3
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 4
                        movdq2q     mmx_temp3, xmm4
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 5
                        movdq2q     mmx_temp3, xmm5
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 6
                        movdq2q     mmx_temp3, xmm6
                        pxor        reg, mmx_temp3
                        ELSEIF      count eq 7
                        movdq2q     mmx_temp3, xmm7
                        pxor        reg, mmx_temp3
                        ELSE
                        .ERR        'Invalid index in XORXSSE2'
                        ENDIF
                        ENDM

TIGERRNDSSE2            MACRO       count:REQ,mulval:REQ
; c = c ^ _x[count]
; a = a - (T1[( c_l    &0ffh)*8]^T2[((c_l>>16)&0ffh)*8)^T3[( c_h    &0ffh)*8]^T4[((c_h>>16)&0ffh)*8]
; b = b + (T4[((c_l>>8)&0ffh)*8]^T3[((c_l>>24)&0ffh)*8)^T2[((c_h>>8)&0ffh)*8]^T1[((c_h>>24)&0ffh)*8]
; b = b * mul

                        XORXSSE2    mmx_c, count

                        movd        reg_temp1, mmx_c
                        pshufw      mmx_temp1, mmx_c, 4EH
                        mov         reg_temp2, reg_temp1
                        and         reg_temp1, 0ffh
                        movd        reg_temp3, mmx_temp1
                        and         reg_temp2, 0ff0000h
                        movq        mmx_temp2, qword ptr [T1+reg_temp1*8]
                        shr         reg_temp2, 16
                        mov         reg_temp4, reg_temp3
                        pxor        mmx_temp2, qword ptr [T2+reg_temp2*8]
                        and         reg_temp3, 0ffh
                        and         reg_temp4, 0ff0000h
                        pxor        mmx_temp2, qword ptr [T3+reg_temp3*8]
                        shr         reg_temp4, 16
                        pxor        mmx_temp2, qword ptr [T4+reg_temp4*8]
                        psubq       mmx_a, mmx_temp2
                        
                        movd        reg_temp1, mmx_c
                        mov         reg_temp2, reg_temp1
                        and         reg_temp1, 0ff00h
                        movd        reg_temp3, mmx_temp1
                        and         reg_temp2, 0ff000000h
                        shr         reg_temp1, 8
                        movq        mmx_temp2, qword ptr [T4+reg_temp1*8]
                        shr         reg_temp2, 24
                        mov         reg_temp4, reg_temp3
                        pxor        mmx_temp2, qword ptr [T3+reg_temp2*8]
                        and         reg_temp3, 0ff00h
                        and         reg_temp4, 0ff000000h
                        shr         reg_temp3, 8
                        pxor        mmx_temp2, qword ptr [T2+reg_temp3*8]
                        shr         reg_temp4, 24
                        pxor        mmx_temp2, qword ptr [T1+reg_temp4*8]
                        paddq       mmx_b, mmx_temp2

                        movq        mmx_temp1, mmx_b
                        IF          mulval eq 5
                        psllq       mmx_b, 2
                        paddq       mmx_b, mmx_temp1
                        ELSEIF      mulval eq 7
                        psllq       mmx_b, 3
                        psubq       mmx_b, mmx_temp1
                        ELSEIF      mulval eq 9
                        psllq       mmx_b, 3
                        paddq       mmx_b, mmx_temp1
                        ELSE
                        .ERR        'invalid mul value in TIGERRNDSSE2'
                        ENDIF

                        ENDM

ROTATEVARSSSE2          MACRO
;tempa=a;a=b;b=c;c=tempa
reg_t                   textequ     mmx_a
mmx_a                   textequ     mmx_b
mmx_b                   textequ     mmx_c
mmx_c                   textequ     reg_t
                        ENDM

TIGERROUNDSSE2          MACRO       mulval:REQ

count                   =           0
                        REPEAT      8
                        TIGERRNDSSE2 count, mulval
count                   =           count + 1
                        ROTATEVARSSSE2
                        ENDM
                        ENDM

TIGERKEYSCHEDULESSE2    MACRO

                        movdq2q     mmx_temp1, xmm7
                        pxor        xmm7, oword ptr const_A5A5A5A5A5A5A5A5
                        psubq       xmm0, xmm7
                        movq2dq     xmm7, mmx_temp1
                        pxor        xmm1, xmm0
                        movdq2q     mmx_temp2, xmm1
                        paddq       xmm2, xmm1
                        pxor        xmm1, oword ptr const_FFFFFFFFFFFFFFFF            ; not
                        psllq       xmm1, 19
                        pxor        xmm1, xmm2
                        psubq       xmm3, xmm1
                        movq2dq     xmm1, mmx_temp2
                        pxor        xmm4, xmm3
                        movdq2q     mmx_temp3, xmm4
                        paddq       xmm5, xmm4
                        pxor        xmm4, oword ptr const_FFFFFFFFFFFFFFFF            ; not
                        psrlq       xmm4, 23
                        pxor        xmm4, xmm5
                        psubq       xmm6, xmm4
                        movq2dq     xmm4, mmx_temp3
                        pxor        xmm7, xmm6
                        movdq2q     mmx_temp4, xmm7
                        paddq       xmm0, xmm7
                        pxor        xmm7, oword ptr const_FFFFFFFFFFFFFFFF            ; not
                        psllq       xmm7, 19
                        pxor        xmm7, xmm0
                        psubq       xmm1, xmm7
                        movq2dq     xmm7, mmx_temp4
                        pxor        xmm2, xmm1
                        movdq2q     mmx_temp5, xmm2
                        paddq       xmm3, xmm2
                        pxor        xmm2, oword ptr const_FFFFFFFFFFFFFFFF            ; not
                        psrlq       xmm2, 23
                        pxor        xmm2, xmm3
                        psubq       xmm4, xmm2
                        movq2dq     xmm2, mmx_temp5
                        pxor        xmm5, xmm4
                        paddq       xmm6, xmm5
                        movdq2q     mmx_temp1, xmm6
                        pxor        xmm6, oword ptr const_0123456789ABCDEF
                        psubq       xmm7, xmm6
                        movq2dq     xmm6, mmx_temp1

                        ENDM

                        ALIGN       16

TigerTree_Tiger_SSE2    PROC        PUBLIC  _Data:DWORD, _State:DWORD
; Compiles a Block of 8 64-bit words that can be found at _Data
                        pusha

__Data                  textequ     <[esp+36]>
__State                 textequ     <[esp+40]>

                        mov         reg_temp5, __Data
                        movdqu      xmm0, [reg_temp5]                        ; we can't guaranty alignment
                        pshufd      xmm1, xmm0, 4EH
                        movdqu      xmm2, [reg_temp5+2*8]
                        pshufd      xmm3, xmm2, 4EH
                        movdqu      xmm4, [reg_temp5+4*8]
                        pshufd      xmm5, xmm4, 4EH
                        movdqu      xmm6, [reg_temp5+6*8]
                        pshufd      xmm7, xmm6, 4EH

                        mov         reg_temp5, __State
                        movq        mmx_a, qword ptr [reg_temp5+m_nState0]
                        movq        mmx_b, qword ptr [reg_temp5+m_nState1]
                        movq        mmx_c, qword ptr [reg_temp5+m_nState2]

                        TIGERROUNDSSE2 5
                        TIGERKEYSCHEDULESSE2
                        TIGERROUNDSSE2 7
                        TIGERKEYSCHEDULESSE2
                        TIGERROUNDSSE2 9

                        pxor        mmx_a, qword ptr [reg_temp5+m_nState0]
                        movq        qword ptr [reg_temp5+m_nState0], mmx_a
                        psubq       mmx_b, qword ptr [reg_temp5+m_nState1]
                        movq        qword ptr [reg_temp5+m_nState1], mmx_b
                        paddq       mmx_c, qword ptr [reg_temp5+m_nState2]
                        movq        qword ptr [reg_temp5+m_nState2], mmx_c

                        emms

                        popa
                        ret 8

TigerTree_Tiger_SSE2    ENDP

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
; end of SSE2 code
; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

                end
