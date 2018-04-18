#!/bin/sh
### file: dvfs_stress_test.sh
### author: xsf@rock-chips.com
### function: cpu dvfs stress test
### date: 20180410

echo "start dvfs test"

# test clks array, unit: KHz, must sort from big to small

#clks0=1200000000
clks0=1008000000
clks1=816000000
clks2=600000000
clks3=408000000

clkstart=1200000000
clks_cnt=5
last_clk=clks5

# test voltage, unit: uV
volt_start=1275000
volt_end=950000
volt_step=25000

# number of threads for dhrystone
threads=4

KHz=1000
volt=$volt_start
clk=$clkstart
# the file to save test result
                                    
while true                                                          
case $clk in
$clkstart)                                                     
        echo "old clock:" $clk                             
        clk=$clks0
        volt=1175000
        volt0=1175000                                        
;;                                                       
$clks0)                                                     
        echo "old clock:" $clk                             
        clk=$clks1
        volt=1000000
        volt1=1000000                                        
;;                                                         
$clks1)                                                     
        echo "old clock:" $clk                             
        clk=$clks2
        volt=975000
        volt2=975000                                         
;;                                                         
$clks2)                                                     
        echo "old clock:" $clk                             
        clk=$clks3
        volt=950000
        volt3=950000                                       
;;                                                         
$clks3)                                                     
        echo "old clock:" $clk 
        echo "dvfs pass"
		echo interactive > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor                     
        exit                                          
;;                                                         
esac                                                   
         echo "now:" $volt $clk                   

sync                                                                

cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor           
cat /sys/kernel/debug/regulator/vdd_arm/voltage
echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo $volt_start > /sys/kernel/debug/regulator/vdd_arm/voltage
cat /sys/kernel/debug/regulator/vdd_arm/voltage 

echo "voltage....................." $volt
echo $clk > /sys/kernel/debug/clk/armclk/clk_rate
sleep 1

  
do 
	echo $volt > /sys/kernel/debug/regulator/vdd_arm/voltage                                                                   
        echo "now test clk:" $clk", volt=" $volt                    
        echo "cat the clock and voltage"                            
        cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor   
        cat /sys/kernel/debug/regulator/vdd_arm/voltage            
        cat /sys/kernel/debug/clk/armclk/clk_rate                   
        echo "end"                                                  
######################## memtester ######################################
echo "start memtester 64bit:"
memtester 1M 5 &
pid=$!
	echo "memtester pid: " $pid


	#wait for memtester done
	i=1
	while true
	do	
		sleep 1
		#proc file is not exist, so memtester has been done
		ls /proc/$pid > /dev/null
		if [ $? -ne 0 ]; then
			break;
		else
			echo "wait for memtester done: " $i " second"
		fi

		i=$((i+1))
		if [ $i -eq 60 ]; then
			break;
		fi
	done
echo "memteseter end!!!!!!!!"

#######################################stressapptest#############################
echo "start stressapptest"

stressapptest -s 500 -i 4 -C 4 -W --stop_on_errors -M 200 --pause_delay 4 &
        pid=$!                                                               
        echo "pid: " $pid 

i=1
while true
do
	echo "wait for stressapptest done: " $i " second"	

                sleep 1                                                      
                #proc file is not exist, so dhrystone has been done          
           #     ls /proc/$pid > /dev/null                                    
            #    if [ $? -ne 0 ]; then                                        
             #           break;                                               
              #  else                                                         
               #         echo "wait for dhrystone done: " $i " second"            
              #  fi               

	sleep 1
	i=$((i+1))
	if [ $i -eq 260 ]; then
		break
	fi
done

############################## record result ####################################
                                                                   

        if [ $clk -eq $clks3 ]; then
		echo interactive > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
        	cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
		echo "dvfs ok...."
		echo "frequency:" $clks0 $clks1 $clks2 $clks3 
		echo "voltage:"   $volt0 $volt1 $volt2 $volt3                                 
                exit                                                             
        fi                                                                       
                                                                                 
############################## start next test ####################################
                                                   
        echo "xxxxxxxxxxxxnow voltage: " $volt                                                 
done
