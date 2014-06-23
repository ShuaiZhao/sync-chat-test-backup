echo 'results/log/topo_10_proxies_300UE_20_5_10GC_10GL_halfSource_136bytes_audio.tr'
time ./waf "--run=sync-simulation --ConfigInfo="config.txt" --TopoInfoFile="topo_10_proxies_300UE_20_5_10GC_10GL.txt" --SimulationTime=6" > results/log/topo_10_proxies_300UE_20_5_10GC_10GL_halfSource_136bytes_audio.tr 2>&1

echo 'results/log/topo_10_proxies_300UE_20_5_10GC_1GL_halfSource_136bytes_audio.tr'
time ./waf "--run=sync-simulation --ConfigInfo="config.txt" --TopoInfoFile="topo_10_proxies_300UE_20_5_10GC_1GL.txt" --SimulationTime=6" > results/log/topo_10_proxies_300UE_20_5_10GC_1GL_halfSource_136bytes_audio.tr 2>&1
