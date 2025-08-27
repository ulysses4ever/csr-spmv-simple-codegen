{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  NIX_ENFORCE_NO_NATIVE = "";
  packages = [
    (pkgs.python3.withPackages (pkgs: with pkgs; [
      numpy
      scipy
      pandas
      matplotlib
    ]))
  ];
}
