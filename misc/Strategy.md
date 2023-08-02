
Strategy                                 opt_design -directive                             place_design -directive
Performance_Explore                      Explore                                           Explore
Performance_ExplorePostRoutePhysOpt      Explore                                           Explore
Performance_ExploreWithRemap             ExploreWithRemap                                  Explore
Performance_WLBlockPlacement             Default                                           WLDrivenBlockPlacement
Performance_WLBlockPlacementFanoutOpt    Default                                           WLDrivenBlockPlacement
Performance_EarlyBlockPlacement          Explore                                           EarlyBlockPlacement
Performance_NetDelay_high                Default                                           ExtraNetDelay_high
Performance_NetDelay_low                 Explore                                           ExtraNetDelay_low
Performance_Retiming                     Default                                           ExtraPostPlacementOpt
Performance_ExtraTimingOpt               Default                                           ExtraTimingOpt
Performance_RefinePlacement              Default                                           ExtraPostPlacementOpt
Performance_SpreadSLLs                   Default                                           SSI_SpreadSLLs
Performance_BalanceSLLs                  Default                                           SSI_BalanceSLLs
Performance_BalanceSLRs                  Default                                           SSI_BalanceSLRs
Performance_HighUtilSLRs                 Default                                           SSI_HighUtilSLRs
Congestion_SpreadLogic_high              Default                                           AltSpreadLogic_high
Congestion_SpreadLogic_medium            Default                                           AltSpreadLogic_medium
Congestion_SpreadLogic_low               Default                                           AltSpreadLogic_low
Congestion_SSI_Spreadlogic_high          Default                                           SSI_SpreadLogic_high
Congestion_SSI_Spreadlogic_low           Default                                           SSI_SpreadLogic_low
Area_Explore                             ExploreArea                                       Default
Area_ExploreSequential                   ExploreSequentialArea                             Default
Area_ExploreWithRemap                    ExploreWithRemap                                  Default
Power_DefaultOpts                        Default                                           Default
Power_ExploreArea                        ExploreSequentialArea                             Default
Flow_RunPhysOpt                          Default                                           Default
Flow_RunPostRoutePhysOpt                 Default                                           Default
Flow_RuntimeOptimized                    RuntimeOptimized                                  RuntimeOptimized
Flow_Quick                               RuntimeOptimized                                  Quick



Strategy                                 phys_opt_design -directive                        route_design -directive
Performance_Explore                      Explore                                           Explore
Performance_ExplorePostRoutePhysOpt      Explore 1                                         Explore
Performance_ExploreWithRemap             Explore                                           NoTimingRelaxation
Performance_WLBlockPlacement             Explore                                           Explore
Performance_WLBlockPlacementFanoutOpt    AggressiveFanoutOpt                               Explore
Performance_EarlyBlockPlacement          Explore                                           Explore
Performance_NetDelay_high                AggressiveExplore                                 NoTimingRelaxation
Performance_NetDelay_low                 AggressiveExplore                                 NoTimingRelaxation
Performance_Retiming                     AlternateFlowWithRetiming                         Explore
Performance_ExtraTimingOpt               Explore                                           NoTimingRelaxation
Performance_RefinePlacement              Default                                           NoTimingRelaxation
Performance_SpreadSLLs                   Explore                                           Explore
Performance_BalanceSLLs                  Explore                                           Explore
Performance_BalanceSLRs                  Explore                                           Explore
Performance_HighUtilSLRs                 Explore                                           Explore
Congestion_SpreadLogic_high              AggressiveExplore                                 AlternateCLBRouting
Congestion_SpreadLogic_medium            Explore                                           AlternateCLBRouting
Congestion_SpreadLogic_low               Explore                                           AlternateCLBRouting
Congestion_SSI_SpreadLogic_high          AggressiveExplore                                 AlternateCLBRouting
Congestion_SSI_SpreadLogic_low           Explore                                           AlternateCLBRouting
Area_Explore                             Not enabled                                       Default
Area_ExploreSequential                   Not enabled                                       Default
Area_ExploreWithRemap                    Not enabled                                       Default
Power_DefaultOpts                        Not enabled                                       Default
Power_ExploreArea                        Not enabled                                       Default
Flow_RunPhysOpt                          Explore                                           Default
Flow_RunPostRoutePhysOpt                 Explore 1                                         Default
Flow_RuntimeOptimized                    Not enabled                                       RuntimeOptimized
Flow_Quick                               Not enabled                                       Quick