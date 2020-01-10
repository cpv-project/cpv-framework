# Benchmarks

This folder manages source code of benchmark targets, and the tool used to generate benchmark results.

## Prepare environment

First, ensure you created development environment from `cpv-ops/local`, then execute `devenv-root-enter.sh` to enter devenv container with root, and run following commands:

``` sh
curl https://packages.microsoft.com/config/ubuntu/18.04/packages-microsoft-prod.deb -o packages-microsoft-prod.deb
dpkg -i packages-microsoft-prod.deb
rm packages-microsoft-prod.deb
add-apt-repository universe
apt-get update
apt-get install -y apt-transport-https python3-psutil
apt-get update
apt-get install -y dotnet-sdk-3.1

su ubuntu
curl -sf -L https://static.rust-lang.org/rustup.sh | sh
exit

cd /tmp
git clone https://github.com/giltene/wrk2
cd wrk2
make
mv wrk /usr/local/bin/wrk2
cd /tmp
git clone https://github.com/wg/wrk
cd wrk
make
mv wrk /usr/local/bin/wrk
```

## Generate benchmark results

Use `devent-enter.sh` from `cpv-ops/local/scripts` to enter devenv container, and run following command:

``` sh
sh run_benchmarks.sh
```

It will perform benchmarks for all targets, and output results in markdown format.
